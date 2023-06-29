#include <iostream>
#include <cmath>
#include <vector>
#include <ff/ff.hpp>
#include <ff/farm.hpp>
#include <ff/parallel_for.hpp>
#include <ff/barrier.hpp>
#include <functional>

#define CHUNKS_PER_WORKER 4

using namespace ff;
using namespace std;

template<typename T>
class StencilPatternParFF {
private:
    std::function<T(std::vector<T>)> stencilFunc;
    std::vector<std::pair<int, int>> neighborhood;
    int iterations;
    int nw;
public:
    StencilPatternParFF(std::function<T(std::vector<T>)> stencilFunc, std::vector<std::pair<int, int>> neighborhood, int iterations, int nw)
    : stencilFunc(stencilFunc), neighborhood(neighborhood), iterations(iterations), nw(nw) {}

    std::vector<std::vector<T>> operator()(std::vector<std::vector<T>> data) { //capture data by reference
        /*
        Here we make two copies of the input stencil matrix. We don't want to change the input data, therefore we
        make two copies of it.
        The idea is to write the output of the stencil function into the data2 matrix, and after every matrix has
        calculated the output, the data1 and data2 matrices are swapped (std::swap). This method wastes twice the
        memory, but is the fastest way to do the calculations, while remaining thread safe. 
        */
        std::vector<std::vector<T>> data1 = data;
        vector<vector<T>> data2 = data1; //copies the 2d matrix?
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
        //Creates the ParallelFor FastFlow block, with nw workers.
        ParallelFor pf(nw, true);
        /*
        In every iteration, a parallel for is ran on all indexes, with dynamic scheduling, so that every thread
        is working while the queue isnt empty
        */
        for (int i=0; i<iterations; i++) {
            pf.parallel_for(0, n_indexes, 1, nw*CHUNKS_PER_WORKER, [&](int index) {
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
            }, nw);
            //matrices are swapped so that the next iteration can build upon the previous one
            std::swap(data1, data2);
        }
        return data1;
    }
};