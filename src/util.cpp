
#include <cmath>
#include <vector>
#include "util.h"

double stencilAvgFunction(std::vector<double> vec) {
	int size = vec.size();
	int sum = 0;
	for (int i = 0; i < size; i++) {
		sum += vec[i];
	}
	double res = sum * 1.0 / size;
	return res;
}

double stencilSinFunction(std::vector<double> vec) {
	int size = vec.size();
	double res = vec[0];
	for (int j = 0; j < 500; j++) {
		for (int i = 0; i < size; i++) {
			res = sin(res);
		}
	}
	
	return res;
}

double stencilUnstableFunction(std::vector<double> vec) {
	double res = vec[0];
	if(res < (max / 2)) {
		//std::this_thread::sleep_for(std::chrono::milliseconds(100)); //sleeps for 0.1 seconds
	} else {
		//std::this_thread::sleep_for(std::chrono::milliseconds(200)); //sleeps for 0.2 seconds
	}
	//cout << "Calculation finished" << endl;
	
	return res;
}