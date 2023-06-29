#include <vector>
#include <functional>
#include <thread>
#include <barrier>
#include <iostream>
#include "queue.cpp"
#include <time.h>

using namespace std;

template<typename T>
class StencilPatternParThreads {
public:
    StencilPatternParThreads(std::function<T(std::vector<T>)> stencilFunc, std::vector<std::pair<int, int>> neighborhood, int iterations, int nworkers)
        : stencilFunc(stencilFunc), neighborhood(neighborhood), iterations(iterations), nworkers(nworkers) {}

    std::vector<std::vector<T>> operator()(std::vector<std::vector<T>> data) {
        /*
        Here we make two copies of the input stencil matrix. We don't want to change the input data, therefore we
        make two copies of it.
        The idea is to write the output of the stencil function into the data2 matrix, and after every matrix has
        calculated the output, the data1 and data2 matrices are swapped (std::swap). This method wastes twice the
        memory, but is the fastest way to do the calculations, while remaining thread safe. 
        */
        std::vector<std::vector<T>> data1 = data;
        std::vector<std::vector<T>> data2 = data1;
        int numRows = data1.size();
        int numCols = data1[0].size();

        /*
        This section of the code calculates the starting and ending lines and columns, given that the borders of
        the stencil matrix are not supposed to be calculated. It iterates through the neighborhood input vector
        and stores the maximum offset of each axis.
        */
        int max_y_offset = 0, max_x_offset = 0, min_y_offset = 0, min_x_offset = 0;
        for (auto offset : neighborhood) {
            int y_offset = offset.first;
            int x_offset = offset.second;

            if (y_offset > max_y_offset) max_y_offset = y_offset;
            if (y_offset < min_y_offset) min_y_offset = y_offset;

            if (x_offset > max_x_offset) max_x_offset = x_offset;
            if (x_offset < min_x_offset) min_x_offset = x_offset;
        }

        //calculation of the start and end row and column
        int start_row = -min_y_offset, end_row = numRows - max_y_offset;
        int start_col = -min_x_offset, end_col = numCols - max_x_offset;

        /*
        Here we calculate the total number of rows, columns, and the total number of indexes to process.
        */
        int rows = end_row - start_row; //number of rows to process
        int cols = end_col - start_col; //number of columns to process
        int n_indexes = rows * cols; //number of total indexes to process

        // we create the vector of threads so that we can join them later
        std::vector<std::thread> threads;
        // we create the queue that will allow us to process the tasks in parallel thread safely
        ThreadSafeQueue<T> tsq;
        auto start = std::chrono::system_clock::now();
        auto stop = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_taskfill;

        //this function is called after all the threads hit the last barrier. At this moment, the 
        // data1 and data2 matrices are swapped, so that the next iteration can build up on the previous
        // iteration.
        //It also fills the task matrix with the neighbor vectors of all indexes.
        auto on_completion = [&]() {
            std::swap(data1, data2);
            start = std::chrono::system_clock::now();
            for (int index = 0; index < n_indexes; index++) {
                int line = index / cols + start_row; //calculate the line index
                int column = index % cols + start_col; //calculate the column index

                //define the neighbor vector
                std::vector<T> neighbors;
                //push the current index
                neighbors.push_back(data1[line][column]); //insert current element in neighbors vec
                //push all the neighbors
                for (auto offset : neighborhood) {
                    int ni = line + offset.first;
                    int nj = column + offset.second;
                    neighbors.push_back(data1[ni][nj]); //insert current neighbor in neighbors vec
                }
                Task<T> t(neighbors, line, column);
                //we push unsafely to the queue because we know for sure only one thread is running this code
                tsq.unsafe_push(t);
            }
            stop = std::chrono::system_clock::now();
            elapsed_taskfill = (stop - start);
            auto musec =
                std::chrono::duration_cast<std::chrono::microseconds>(elapsed_taskfill).count();
            std::cout << "Time to fill task queue computed in " << musec << " usec "
                << std::endl;
            
        };

        /*
        Definition of the barrier.
        The barrier happens after all threads made the computation of all tasks of the queue. This is needed
        so that we can swap the matrices and fill the task queue. This is done with the on_completion
        function that is called after all threads have been gathered by the barrier
        */
        std::barrier sync_threads(nworkers, on_completion);

        /*
        Code of each worker thread
        Every thread pops tasks from the queue (thread safely), and processes the result of the
        stencil function.
        */
        auto worker = [&]() {
            for (int it = 0; it < iterations; it++) {
                /*
                The barrier swaps the data1 and data2 matrices, so that every iteration can build upon the
                previous iteration.
                It also fills the task queue with all the indexes that need to be computed
                */
                cout << clock() << endl;
                sync_threads.arrive_and_wait();
                //cout << std::chrono::system_clock::now() << endl;
                Task<T> t;
                /*
                While the queue is not empty, a task is popped, and the result of the stencil function is placed
                in the buffer matrix
                */
                while(tsq.pop(t)) {
                    data2[t.getLine()][t.getCol()] = stencilFunc(t.getNei());
                }
            }
        };

        start = std::chrono::system_clock::now();
        //launches the threads to do the work
        for (int i = 0; i < nworkers-1; i++) {
            threads.push_back(std::thread(worker));
        }
        //and puts the main execution to work aswell, so that no thread goes to waste!
        worker();

        //destroys all the threads that were launched
        for (auto& thread : threads) {
            thread.join();
        }
        stop = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed =
            (stop - start) - elapsed_taskfill;
        auto musec =
            std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
        std::cout << "Overhead to pop and compute all tasks is " 
        << musec << " usec " << std::endl;

        //returns final matrix
        return data2;
    }



private:
    std::function<T(std::vector<T>)> stencilFunc; //stencil function to be applied on each neighborhood vector
    std::vector<std::pair<int, int>> neighborhood; //neighborhood offset positions
    int iterations;
    int nworkers;
};