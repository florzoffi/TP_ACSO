/**
 * File: thread-pool.cc
 * --------------------
 * Presents the implementation of the ThreadPool class.
 */

#include "thread-pool.h"
#include <stdexcept>
using namespace std;

ThreadPool::ThreadPool( size_t numThreads ) : wts( numThreads ), done( false ) {
    dt = thread( [this] { dispatcher(); } );

    for ( size_t i = 0; i < numThreads; ++i ) {
        wts[i].ts = thread( [this, i] { worker( i ); } );
    }
}

void ThreadPool::dispatcher() {
    while ( true ) {
        unique_lock<mutex> lock( queueLock );

        taskAvailableCV.wait( lock, [this]() { return !taskQueue.empty() || done; } );
        if ( done && taskQueue.empty() ) { return; }

        size_t workerId = 0;
        bool found = false;

        while (!found) {
            for (size_t i = 0; i < wts.size(); ++i) {
                lock_guard<mutex> guard(wts[i].mtx); 
                if (!wts[i].busy) {
                    workerId = i;
                    wts[i].busy = true;
                    found = true;
                    break;
                }
            }
            if (!found) {
                lock.unlock();
                this_thread::yield();
                lock.lock();
            }
        }
        {
            lock_guard<mutex> guard(wts[workerId].mtx);
            wts[workerId].thunk = taskQueue.front();
        }
        taskQueue.pop();
        activeTasks++;

        lock.unlock();
        wts[workerId].sem.signal();
    }
}

void ThreadPool::worker( int id ) {
    while ( true ) {
        wts[id].sem.wait();

        if ( done && !wts[id].thunk ) break;

        function<void()> job;
        {
            lock_guard<mutex> guard(wts[id].mtx); // 🚨 acceso protegido
            job = wts[id].thunk;
            wts[id].thunk = nullptr;
            wts[id].busy = false;
        }

        if (done && !job) break;
        job();

        activeTasks--;
        if ( activeTasks == 0 && taskQueue.empty() ) { noTasksLeftCV.notify_all(); }
    }
}

void ThreadPool::schedule( const function<void( void )>& thunk ) {
    if ( destroyed ) {
        throw runtime_error( "Cannot schedule after ThreadPool is destroyed" );
    }
    if ( !thunk ) {
        throw invalid_argument( "Cannot schedule a null function" );
    }
    {
        lock_guard<mutex> lock( queueLock );
        taskQueue.push( thunk );
    }
    taskAvailableCV.notify_all();
}

void ThreadPool::wait() {
    lock_guard<mutex> outer( waitMutex );
    unique_lock<mutex> lock( queueLock );
    noTasksLeftCV.wait( lock, [this]() { return activeTasks == 0 && taskQueue.empty(); } );
}

ThreadPool::~ThreadPool() {
    wait(); 

    {
        lock_guard<mutex> lock( queueLock );
        done = true;
    }

    taskAvailableCV.notify_all();
    if ( dt.joinable() ) dt.join();

    for ( auto& w : wts ) {
        w.sem.signal();    
        if ( w.ts.joinable() ) w.ts.join();
    }
    destroyed = true;
}
