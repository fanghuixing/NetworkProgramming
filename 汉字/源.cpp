#include <iostream>
using namespace std;

//s是原字符串，o是需要找到的字
//找到一个字即返回
//true表示找到了，false表示没找到
bool search(unsigned char *s, unsigned char *o)
{
	while (*o) {
		if (*s != *o)
		{
			return false;
		}
		else {
			s++;
			o++;
		}
	}
	return true;
}

int main()
{
	unsigned char  youhanzi[100] = "ab213131你213131dasada你0000你ss";	
	unsigned char ni[10] = "你";

	//从youhanzi中找到ni

	unsigned char * start = youhanzi;
	int amount = 0;
	while (*start)
	{
		if (search(start, ni))
		{
			amount++;
			start = start + strlen((char *)ni);
			//找到一个ni后，偏移（ni的长度）指针，然后找下一个ni
		}
		else {
			//如果没找到ni，就偏移下一个位置
			start++;
		}
	}

	cout << amount;
	return 0;
}

