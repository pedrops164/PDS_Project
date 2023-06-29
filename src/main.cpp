#include <iostream>
#include <vector>
#include <cmath>
#include "sequential.cpp"
#include "new_par_threads.cpp"
#include "par_fastflow.cpp"
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
	vector<vector<double>> par_threads;
	vector<vector<double>> par_ff;
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
	

	// Parallel implementation time using C++ native threads
	{
		utimer t0("parallel time", runs);

		for (int i=0; i<runs; i++) {
			NewStencilPatternParThreads<double> sp(function, neighborhood, iterations, nworkers);
			par_threads = sp(data);
		}		
	}
	if (printMatrix) {
		for (int i = 0; i < lines; i++) {
			for (int j = 0; j < columns; j++) {
				cout << par_threads[i][j] << "  ";
			}
			cout << endl;
		}
	}

	//// Parallel implementation time using FastFlow
	{
		utimer t0("parallel time fastflow", runs);
//
		for (int i=0; i<runs; i++) {
			StencilPatternParFF<double> sp(function, neighborhood, iterations, nworkers);
			par_ff = sp(data);
		}		
	}
	if (printMatrix) {
		for (int i = 0; i < lines; i++) {
			for (int j = 0; j < columns; j++) {
				cout << par_ff[i][j] << "  ";
			}
			cout << endl;
		}
	}

	for (int i=0; i<lines; i++) {
		for (int j=0; j<columns; j++) {
			if ((seq[i][j] != par_threads[i][j]) || (par_threads[i][j] != par_ff[i][j])) {
				cout << "The three computations don't output equal matrices" << endl;
				cout << i << "," << j << endl;
				return -1;
			}
		}
	}
	cout << "The three computations output equal matrices\nThe computation was correct" << endl;	
	return 0;
}
