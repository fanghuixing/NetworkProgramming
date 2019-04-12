// read a file into memory
#include <iostream>     // std::cout
#include <fstream>      // std::ifstream
using std::ios;
using std::ifstream;
using std::cout;
int main() {
	ifstream is("test.txt", ios::binary);
	if (is) {
		printf("file opened!\n");
		// get length of file:
		is.seekg(0, is.end);
		int length = is.tellg();
		is.seekg(0, is.beg);

		// allocate memory:
		char * buffer = new char[length];

		// read data as a block:
		is.read(buffer, length);

		is.close();

		// print content:
		cout.write(buffer, length);

		delete[] buffer;
	}

	return 0;
}