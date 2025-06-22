/**
 * File: thread-pool.cc
 * --------------------
 * Presents the implementation of the ThreadPool class.
 */

#include "thread-pool.h"
#include <stdexcept>
using namespace std;

ThreadPool::ThreadPool( size_t numThreads ) : wts( numThreads ), done( false ) {
    for (size_t i = 0; i < numThreads; ++i) {
        wts[i].ts = thread([this, i] { worker( i ); });
    }
    dt = thread( [this] { dispatcher(); } );
}

void ThreadPool::worker( int id ) {
    while (true) {

        wts[id].sem.wait();

        function<void()> task;
        {
            lock_guard<mutex> lock(queueLock);
            task = wts[id].thunk;
            wts[id].thunk = nullptr;
            wts[id].busy = false;
        }

        if (!task) {
            if (done) return;
            continue;
        }

        task();

        {
            lock_guard<mutex> lock(queueLock);
            activeTasks--;
            if (activeTasks == 0 && taskQueue.empty()) {
                noTasksLeftCV.notify_all();
            }
        }
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
    unique_lock<mutex> lock( queueLock );
    noTasksLeftCV.wait( lock, [this]() { return activeTasks == 0 && taskQueue.empty(); } );
}

ThreadPool::~ThreadPool() {
    {
        lock_guard<mutex> lock(queueLock);
        done = true;
    }

    taskAvailableCV.notify_all();

    for (auto& w : wts) {
        w.sem.signal(); 
        if (w.ts.joinable()) w.ts.join();
    }

    destroyed = true;
}

void ThreadPool::dispatcher() {
    while ( true ) {
        unique_lock<mutex> lock( queueLock );
        taskAvailableCV.wait( lock, [this]() {
            return !taskQueue.empty() || done;
        } );

        if ( done && taskQueue.empty() ) return;
        if (taskQueue.empty()) continue;

        size_t workerId = 0;
        bool found = false;

        while ( !found ) {
            if ( done ) return;
            for ( size_t i = 0; i < wts.size(); ++i ) {
                if (!wts[i].busy) {
                    workerId = i;
                    wts[i].busy = true;
                    found = true;
                    break;
                }
            }
            if ( !found ) {
                lock.unlock();
                this_thread::yield();
                lock.lock();
            }
        }

        function<void()> task = taskQueue.front();
        taskQueue.pop();
        activeTasks++;
        wts[workerId].thunk = task;
        lock.unlock();
        wts[workerId].sem.signal();
    }
}