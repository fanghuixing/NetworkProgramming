#include <fstream>
#include <iostream>
#include <string>

using std::fstream;
using std::ios;
using std::string;
int main()
{
	fstream os;

	os.open("outfile", ios::trunc | ios::in | ios::out | ios::binary);
	string s = "hello, world";
	os.write("hello, world", 12);
	os.seekp(7);
	os << 'W';
	os.seekp(0, ios::end);
	os << '!';
	os.seekp(0);
	os << 'H';

	char * str = new char[13];
	os.seekg(0, ios::beg);
	os.read(str, 13);
	std::cout << str << '\n';

	os.close();
/*
	fstream is;
	is.open("outfile", ios::in | ios::binary);
	char * str = new char[13];
	is.seekg(0, ios::beg);
	is.read(str, 13);
	std::cout << str << '\n';
	is.close();
	*/
}