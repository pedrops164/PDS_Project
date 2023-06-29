#include <mutex>
#include <queue>
#include <iostream>
#include <memory>

using namespace std;

class Chunk {
private:
    int start; //start index (including)
    int stop; //stop index (excluding)
public:
    Chunk(): start(-1), stop(-1) {}
    Chunk(int start, int stop): start(start), stop(stop) {}
    int getStart() {return start;}
    int getStop() {return stop;}
};

class ThreadSafeQueue {
private:
    //queue filled with pointers to chunks (vectors of tasks)
    std::queue<Chunk> q;
    std::mutex m;
public:
    ThreadSafeQueue() {}

    ThreadSafeQueue(const ThreadSafeQueue& copy): q(copy.q), m() {
    }

    void push(Chunk chunk) {
        //chunk is a copy of the vector passed as argument
        //locks the mutex so that the access to the queue is thread safe
        std::lock_guard<std::mutex> lock(m);
        q.push(chunk);
    }

    //void unsafe_push(std::vector<Task<T>> &chunk) {
    //    //this function pushes a task to the queue unsafely,
    //    //because it doesn't lock the mutex.
    //    // only to be used if the user knows what he's doing
    //    q.push(std::move(chunk));
    //}

    bool pop(Chunk& chunk) {
        /*
        First locks the mutex so that the access to the queue is thread safe
        Then if the queue is empty returns false, so that the thread who calls knows
        If the queue isn't empty it return true and sets the task to the top of the queue.
        */

        std::unique_lock<std::mutex> lock(m);
        // Wait until the queue is not empty or stopFlag is true

        if (q.empty()) {
            return false; // Stop popping if stopFlag is true
        }

        chunk = q.front(); //std::move() ?
        q.pop();
        return true;
    }

    ThreadSafeQueue& operator=(const ThreadSafeQueue& copy) {
        q = copy.q;
        return *this;
    }
};