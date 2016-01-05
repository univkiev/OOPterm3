// lab3 (Gaussian elimination)
#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <algorithm>

using std::vector;
using std::ifstream;


class GaussianElimination {
	vector<vector<double> > matrix;
	static const double EPSILON;

	int threadNumber;
	int columnNumber;
	int rowNumber;

	vector<int> findLimits(int, int);
	static void threadHandler(int, int, int, GaussianElimination *);

	double computeCoeff(int, int);

public:
	GaussianElimination(ifstream &input);
	void print();

	void setThreadNumber(int num) { threadNumber = num; }
	void eliminate();
};

const double GaussianElimination::EPSILON = 0.0001;

double GaussianElimination::computeCoeff(int rowCounter, int columnCounter) {
	return (matrix[rowCounter][columnCounter] / matrix[columnCounter][columnCounter]);
}

vector<int> GaussianElimination::findLimits(int beg, int end) {
	vector<int> delim;
	auto size = end - beg;

	auto items = size / threadNumber;
	auto otherItems = size % threadNumber;

	delim.push_back(beg);

	auto anotherEnd = beg + items + std::min(1, otherItems);
	delim.push_back(anotherEnd);

	for (auto i = 0; i < threadNumber - 1; i++) {
		otherItems = std::max(0, otherItems - 1);
		anotherEnd += items + std::min(1, otherItems);
		delim.push_back(anotherEnd);
	}

	return delim;
}


void GaussianElimination::threadHandler(int i, int beg, int end, GaussianElimination *obj) {
	double coeff;

	for (auto j = i + 1; j < obj->rowNumber; j++) {

		if (fabs(obj->matrix[i][i] - 0) < EPSILON) {
			std::cout << "System has no solution" << std::endl;
			exit(0);
		}

		coeff = obj->computeCoeff(j, i);

		for (auto k = 0; k < obj->columnNumber; k++)
			obj->matrix[j][k] -= obj->matrix[i][k] * coeff;
	}
}


void GaussianElimination::eliminate() {
	vector<std::thread> thVector;
	
	for (auto i = 0; i < columnNumber; i++) {
		auto limits = findLimits(i + 1, rowNumber);

		// assign to each thread his own part of the matrix 
		for (auto j = 0; j < threadNumber; j++) {
			thVector.push_back(std::thread(&threadHandler, i, limits.at(j), limits.at(j + 1), this));
		}

		// synchronize threads
		for (auto &joiner : thVector) 
			joiner.join();
		
		thVector.resize(0);
	}

	// make diagonal elements equal 1
	for (auto i = 0; i < rowNumber; i++)
		for (auto j = 0; j < columnNumber; j++)
			matrix[i][j] /= matrix[i][i];
}

// read matrix from file
GaussianElimination::GaussianElimination(ifstream &input) {
	input >> rowNumber >> columnNumber;
	
	matrix.resize(rowNumber);
	for (auto i = 0; i < rowNumber; i++)
		matrix[i].resize(columnNumber);

	for (auto y = 0; y < rowNumber; y++) 
		for (auto x = 0; x < columnNumber; x++) 
			input >> matrix[y][x];
}


void GaussianElimination::print() {
	std::cout << "***********************";
	
	for (auto i = 0; i < rowNumber; i++) {
		std::cout << std::fixed << std::endl;

		for (auto j = 0; j < columnNumber; j++) {
			if ((fabs(matrix[i][j] - 0.0) < EPSILON))
				std::cout << std::fixed << fabs(matrix[i][j]) << ' ';
			else
				std::cout << std::fixed << matrix[i][j] << ' ';
		}
	}

	std::cout << "\n***********************" << std::endl;
}


int main() {
	std::ifstream input("matrix.txt");
	GaussianElimination inst(input);
	std::cout << "Matrix" << std::endl; 
	inst.print();

	int num;
	std::cout << "\nEnter the number of threads: ";
	std::cin >> num;

	inst.setThreadNumber(num);

	inst.eliminate();
	inst.print();

	return 0;
}