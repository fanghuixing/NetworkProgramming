#include <iostream>
using namespace std;

//s��ԭ�ַ�����o����Ҫ�ҵ�����
//�ҵ�һ���ּ�����
//true��ʾ�ҵ��ˣ�false��ʾû�ҵ�
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
	unsigned char  youhanzi[100] = "ab213131��213131dasada��0000��ss";	
	unsigned char ni[10] = "��";

	//��youhanzi���ҵ�ni

	unsigned char * start = youhanzi;
	int amount = 0;
	while (*start)
	{
		if (search(start, ni))
		{
			amount++;
			start = start + strlen((char *)ni);
			//�ҵ�һ��ni��ƫ�ƣ�ni�ĳ��ȣ�ָ�룬Ȼ������һ��ni
		}
		else {
			//���û�ҵ�ni����ƫ����һ��λ��
			start++;
		}
	}

	cout << amount;
	return 0;
}

