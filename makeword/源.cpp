#include <Windows.h>
#include <iostream>
#include <bitset>
using std::bitset;
using std::cout;
int main() {

	WORD value = MAKEWORD(2, 1);
	bitset<16> bitvec(value);

	std::string mystring =
		bitvec.to_string<char, std::string::traits_type, std::string::allocator_type>();

	cout << mystring;
	
	return 0;
}