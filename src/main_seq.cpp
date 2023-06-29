#include <iostream>
#include <vector>
#include <cmath>
#include <thread>
#include "sequential.cpp"
#include "utimer.h"
#include "util.h"

using namespace std;

int main(int argc, char* argv[]) {
	if (argc < 7) {
		cout << "Wrong usage. Use ./prog seed n nw iterations printMatrix runs" << endl;
		return -1;
	}
	int seed = atoi(argv[1]);
	int n = atoi(argv[2]);
	int nworkers = atoi(argv[3]);
	int iterations = atoi(argv[4]);
	int printMatrix = atoi(argv[5]);
	int runs = atoi(argv[6]);
	int lines = n;
	int columns = n;

	auto function = stencilAvgFunction;

	srand(seed);

	vector<vector<double>> data(lines, vector<double>(columns, 0));
	for (int i = 0; i < lines; i++) {
		for (int j = 0; j < columns; j++) {
			data[i][j] = (double) (rand() % max);
		}
	}

	std::vector<std::pair<int, int>> neighborhood = {
		pair<int,int>(-1,0),
		pair<int,int>(1,0),
		pair<int,int>(0,1),
		pair<int,int>(0,-1)
	};

	vector<vector<double>> seq;
	 //Sequential implementation time
	{
		utimer t0("sequential time", runs);

		for (int i=0; i<runs; i++) {
			StencilPatternSeq<double> sp(function, neighborhood, iterations);
			seq = sp.operator()(data);
		}
	}
	if (printMatrix) {
		for (int i = 0; i < lines; i++) {
			for (int j = 0; j < columns; j++) {
				cout << seq[i][j] << "  ";
			}
			cout << endl;
		}
	}
		return 0;
}
