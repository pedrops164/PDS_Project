#include <vector>
#include <functional>

template<typename T>
class StencilPatternSeq {
public:
    StencilPatternSeq(std::function<T(std::vector<T>)> stencilFunc, std::vector<std::pair<int, int>> neighborhood, int iterations)
    : stencilFunc(stencilFunc), neighborhood(neighborhood), iterations(iterations) {}


    std::vector<std::vector<T>> operator()(const std::vector<std::vector<T>>& data) {
        /*
        Here we make two copies of the input stencil matrix. We don't want to change the input data, therefore we
        make two copies of it.
        The idea is to write the output of the stencil function into the data2 matrix, and after every matrix has
        calculated the output, the data1 and data2 matrices are swapped (std::swap). This method wastes twice the
        memory, but is the fastest way to do the calculations, while remaining thread safe. 
        */
        std::vector<std::vector<T>> data1 = data;
        std::vector<std::vector<T>> data2 = data1;
        int numRows = data.size();
        int numCols = data[0].size();
        /*
        This section of the code calculates the starting and ending lines and columns, given that the borders of
        the stencil matrix are not supposed to be calculated. It iterates through the neighborhood input vector
        and stores the maximum offset of each axis.
        */
        int max_y_offset = 0, max_x_offset = 0, min_y_offset = 0, min_x_offset = 0;
        for (const auto& offset : neighborhood) {
            int y_offset = offset.first;
            int x_offset = offset.second;
            if (y_offset > max_y_offset) max_y_offset = y_offset;
            if (y_offset < min_y_offset) min_y_offset = y_offset;
            if (x_offset > max_x_offset) max_x_offset = x_offset;
            if (x_offset < min_x_offset) min_x_offset = x_offset;
        }
        /*
        This section of the code runs all the iterations in a sequential way
        */
        for (int iter = 0; iter < iterations; ++iter) {
            for (int i = -min_y_offset; i < numRows-max_y_offset; ++i) {
                for (int j = -min_x_offset; j < numCols-max_x_offset; ++j) {
                    //vector of neighbors is created
                    std::vector<T> neighbors;
                    //the current item is taken into account
                    neighbors.push_back(data1[i][j]);
                    //every neighbor is added to the vector of neighbors
                    for (const auto& offset : neighborhood) {
                        int ni = i + offset.first;
                        int nj = j + offset.second;
                        neighbors.push_back(data1[ni][nj]);
                    }
                    //the result of the stencil function is stored in the buffer matrix
                    data2[i][j] = stencilFunc(neighbors);
                }
            }
            //the matrices are swapped so that the next iteration builds up on the computed values
            std::swap(data1,data2);
        }
        return data1;
    }
private:
    std::function<T(std::vector<T>)> stencilFunc; //stencil function to be applied on each neighborhood vector
    std::vector<std::pair<int, int>> neighborhood; //neighborhood offset positions
    int iterations;
};