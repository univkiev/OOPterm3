// matrix game
#include <iostream>
#include <ctime>

using std::cout;
using std::endl;
using std::cin;

const int N = 3; // matrix size

class MatrixGame {
	int matrix[N][N];
	int firstRes[N]; // result array (after a strategy of the first player)
	int secondRes[N]; // result array (after a strategy of the second player)

	// counters of strategies
	int firstCounter[N];
	int secondCounter[N];

	// string/column index 
	int currentIndex[2];

	// uses as a game number counter
	int iterCount;

	void computation();
	void chooseFirst();
	void findAndCompare();

public:
	MatrixGame();

	void interaction();
};

MatrixGame::MatrixGame() {

	for (int i = 0; i < N; i++) {
		firstRes[i] = 0;
		secondRes[i] = 0;

		firstCounter[i] = 0;
		secondCounter[i] = 0;
	}

	currentIndex[0] = 0;
	currentIndex[1] = 0;
}

void MatrixGame::chooseFirst() {
	srand(time(NULL));

	// first strategy of "player 1" will be randomly chosen
	currentIndex[0] = rand() % N;

	for (auto i = 0; i < N; i++)
		secondRes[i] = matrix[currentIndex[0]][i];

	// increment strategies counter
	++firstCounter[currentIndex[0]];

	// second player must choose the most profitable strategy
	auto min = INT64_MAX;
	for (auto i = 0; i < N; i++)
		if (secondRes[i] < min) {
			min = secondRes[i];
			currentIndex[1] = i;
		}

	for (auto i = 0; i < N; i++)
		firstRes[i] = matrix[i][currentIndex[1]];

	++secondCounter[currentIndex[1]];
}

// filling in and wrapping handler
void MatrixGame::interaction() {
	int temp;

	cout << "We have a " << N << "x" << N << " matrix, now you should fill it in: " << endl;
	for (auto i = 0; i < N; i++)
		for (auto j = 0; j < N; j++) {
			cin >> temp;
			matrix[i][j] = temp;
		}

	cout << "Well, how many iterations (games) do you want?: ";
	cin >> iterCount;

	cout << "Okay, I'm computing..." << endl;
	computation();
}

void MatrixGame::computation() {
	auto max = INT64_MIN;
	auto min = INT64_MAX;

	chooseFirst(); // choose first strategy of both players

	// "play" fictive games
	for (auto i = 0; i < iterCount - 1; i++)
		findAndCompare();

	// find max and min values (for finding the price of the game)
	for (auto i = 0; i < N; i++) {
		if (max < firstRes[i])
			max = firstRes[i];

		if (min > secondRes[i])
			min = secondRes[i];
	}

	cout << "Approximate game price = " << (max + min) << "/" << (iterCount * 2) << endl;

	cout << "Approximate frequencies of the first player's strategies: ";
	for (int i = 0; i < N; i++)
		cout << firstCounter[i] << "/" << iterCount << ' ';

	cout << "\nApproximate frequencies of the second player's strategies: ";
	for (int j = 0; j < N; j++)
		cout << secondCounter[j] << "/" << iterCount << ' ';

	cout << endl;
}

void MatrixGame::findAndCompare() {
	auto max = INT64_MIN;
	auto min = INT64_MAX;

	// find the most profitable strategy for the first player
	for (auto i = 0; i < N; i++)
		if (firstRes[i] > max) {
			max = firstRes[i];
			currentIndex[0] = i;
		}


	// update result array and strategies counter
	++firstCounter[currentIndex[0]];

	for (auto i = 0; i < N; i++)
		secondRes[i] += matrix[currentIndex[0]][i];


	// find the most profitable strategy for the second player
	for (auto i = 0; i < N; i++)
		if (secondRes[i] < min) {
			min = secondRes[i];
			currentIndex[1] = i;
		}
	
	++secondCounter[currentIndex[1]];

	for (auto i = 0; i < N; i++)
		firstRes[i] += matrix[i][currentIndex[1]];

}

int main() {
	MatrixGame game;
	game.interaction();

	return 0;
}











