#include <vector>
#include <functional>
#include <thread>
#include <barrier>
#include <iostream>
#include "new_queue.cpp"

#define CHUNKS_PER_WORKER 4

using namespace std;

template<typename T>
class NewStencilPatternParThreads {
public:
    NewStencilPatternParThreads(std::function<T(std::vector<T>)> stencilFunc, std::vector<std::pair<int, int>> neighborhood, int iterations, int nworkers)
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
        int number_of_chunks = nworkers*CHUNKS_PER_WORKER;

        int chunk_size = n_indexes / number_of_chunks;

        // we create the vector of threads so that we can join them later
        std::vector<std::thread> threads;
        // we create the queue that will allow us to process the tasks in parallel thread safely
        ThreadSafeQueue all_chunks;


        //push chunks to the queue
        for (int c=0; c<number_of_chunks; c++) {
            //push all the chunks to the thread access safe queue
            int start = c*chunk_size;
            int stop = (c+1)*chunk_size;
            if (c==number_of_chunks-1) stop = n_indexes;
            Chunk currentChunk(start,stop);
            all_chunks.push(currentChunk);
        }
        ThreadSafeQueue all_chunks_aux = all_chunks;

        // at this point, all_chunks_aux is reset to all the chunks
        auto on_completion = [&]() {
            std::swap(data1, data2);
            all_chunks_aux = all_chunks;
        };
        /*
        Definition of the barrier.
        The barrier happens after all threads made the computation of all tasks of the queue. This is needed
        so that we can swap the matrices and fill the task queue. This is done with the on_completion
        function that is called after all threads have been gathered by the barrier
        */
        std::barrier b(nworkers, on_completion);

        /*
        Code of each worker thread
        Every thread pops tasks from the queue (thread safely), and processes the result of the
        stencil function.
        */
        auto worker = [&]() {

            for (int it = 0; it < iterations; it++) {
                /*
                While the queue is not empty, a task is popped, and the result of the stencil function is placed
                in the buffer matrix
                */
                Chunk chunk;
                while(all_chunks_aux.pop(chunk)) {
                    int start = chunk.getStart();
                    int end = chunk.getStop();
                    for (int index=start; index<end; index++) {
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
                        //The result of the stencil function is placed in the buffer matrix
                        data2[line][column] = stencilFunc(neighbors);
                    }
                }

                /*
                Barrier syncs the threads after they have computed the chunks
                It waits for all computations to be done, so that the next iteration
                can build upon the previous one, by swapping the matrices
                */
                b.arrive_and_wait();
            }
        };

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

        //returns final matrix
        return data1;
    }

    

private:
    std::function<T(std::vector<T>)> stencilFunc; //stencil function to be applied on each neighborhood vector
    std::vector<std::pair<int, int>> neighborhood; //neighborhood offset positions
    int iterations;
    int nworkers;
};