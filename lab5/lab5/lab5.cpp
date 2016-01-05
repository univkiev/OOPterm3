// lab 5 (a tree based on list)
#include <iostream>
#include <vector>

class List {
	std::vector<int> array;

public:
	void remove();
	void insert(int, int);
	void print();

	int sizeOfArray() { return array.size(); }
	int byIndex(int ind) { return array[ind]; }
	void swap(int first, int second) { std::swap(array[first], array[second]); }
};

void List::remove() {
	if (array.size() > 0)
		array.pop_back();
}

void List::insert(int key, int pos) {

	if (pos == -1) {	// default param, insert at last
		array.push_back(key);
		return;
	}

	if (pos < 0 || pos > (int)array.size())
		return;
	
	// insert according to position
	array.insert(array.begin() + pos, key);
	return;
}

void List::print() {
	for (auto iter = array.begin(); iter != array.end(); ++iter)
		std::cout << *iter << ' ';

	std::cout << std::endl;
}


class Heap {
	List list;

	void maxHeapify(int);
	void buildMaxHeap();

	void insert(int, int);
	bool remove(); // remove the largest (root) element

	void print();

public:
	void wrapper();
};

// fix heap properties (after insert/remove)
void Heap::maxHeapify(int index) {
	auto left = 2 * index + 1; // left child's index
	auto right = 2 * index + 2; // right child's index
	auto len = list.sizeOfArray();
	int largest;

	// find the largest and swap with current (if current != largest)
	// then call this function recursive

	if (left < len && list.byIndex(left) > list.byIndex(index))
		largest = left;
	else
		largest = index;

	if (right < len && list.byIndex(right) > list.byIndex(index))
		largest = right;

	if (largest != index) {
		list.swap(index, largest);
		maxHeapify(largest);
	}
}

void Heap::buildMaxHeap() {
	auto len = list.sizeOfArray();

	// we can skip half of nodes because they are leafs
	for (auto i = len / 2; i >= 0 ; i--)
		maxHeapify(i);
}

void Heap::insert(int key, int pos) {
	return list.insert(key, pos);
}

// usually this function is written in priority queue, but my teacher wanted to see remove function like that
bool Heap::remove() {
	auto len = list.sizeOfArray();

	if (len == 0)
		return false;

	list.swap(0, len - 1);
	list.remove();
	maxHeapify(0);

	return true;
}

void Heap::print() {
	list.print();
}

// just an input's handler
void Heap::wrapper() {
	int count, pos, temp;
	char answer;
	
	std::cout << "Enter the count of elements: ";
	std::cin >> count;

	std::cout << "Enter the elements and position (optional, -1 - insert at last)" << std::endl;
	for (auto i = 0; i < count; i++) {
		std::cin >> temp >> pos;
		this->insert(temp, pos);
	}

	std::cout << "Array before building: ";
	this->print();

	this->buildMaxHeap();
	std::cout << "Heap: ";
	this->print();

	while (true) {
		std::cout << "Do you want to remove the topmost node? (y/n): ";
		std::cin >> answer;

		if (answer == 'y') {
			this->remove();
			std::cout << "After removing: ";
			this->print();
		}
		else
			break;
	}

}

int main() {
	Heap *heap = new Heap;
	heap->wrapper();

	return 0;
}









