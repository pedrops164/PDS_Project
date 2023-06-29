#include <mutex>
#include <queue>

template<typename T>
class Task {
private:
    std::vector<T> neighborhood;
    int line;
    int column;
public:
    Task() {}
    Task(std::vector<T> neighborhood, int line, int column):
    neighborhood(neighborhood), line(line), column(column) {}
    int getLine() {
        return line;
    }
    int getCol() {
        return column;
    }
    std::vector<T> getNei() {
        return neighborhood;
    }
};

template<typename T>
class ThreadSafeQueue {
private:
    std::queue<Task<T>> q;
    std::mutex m;
public:
    void push(Task<T> task) {
        //locks the mutex so that the access to the queue is thread safe
        std::lock_guard<std::mutex> lock(m); 
        q.push(std::move(task));
    }

    void unsafe_push(Task<T> task) {
        //this function pushes a task to the queue unsafely,
        //because it doesn't lock the mutex.
        // only to be used if the user knows what he's doing
        q.push(std::move(task));
    }

    bool pop(Task<T>& task) {
        /*
        First locks the mutex so that the access to the queue is thread safe
        Then if the queue is empty returns false, so that the thread who calls knows
        If the queue isn't empty it return true and sets the task to the top of the queue.
        */
        std::lock_guard<std::mutex> lock(m); 
        if (q.empty()) {
            return false;
        }
        task = std::move(q.front());
        q.pop();
        return true;
    }
};