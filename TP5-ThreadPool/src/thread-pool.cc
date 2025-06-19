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
        wts[i].ts = thread([this, i] { worker(i); });
    }
}

void ThreadPool::worker( int id ) {
    while (true) {
        function<void()> task;
        {
            unique_lock<mutex> lock(queueLock);
            taskAvailableCV.wait(lock, [this]() {
                return !taskQueue.empty() || done;
            });
            if (done && taskQueue.empty()) return;
            task = taskQueue.front();
            taskQueue.pop();
            activeTasks++;
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
    lock_guard<mutex> outer( waitMutex );
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
        if (w.ts.joinable()) w.ts.join();
    }

    destroyed = true;
}
