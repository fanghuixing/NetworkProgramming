#include <iostream>
using namespace std;

//IPͷ���ṹ
typedef struct IP_HEAD
{
	union
	{
		unsigned char Version; //�汾(�ֽ�ǰ4λ)
		unsigned char HeadLen; //ͷ������(�ֽں�4λ)
	};

	unsigned char ServiceType;		//��������

	unsigned short TotalLen;		//�ܳ���

	unsigned short Identifier;		//��ʶ��

	union
	{
		unsigned short Flags;		//��־λ(��ǰ3λ)
		unsigned short FragOffset;	//Ƭƫ��(�ֺ�13λ)
	};

	unsigned char TimeToLive;		//��������

	unsigned char Protocol;			//Э��

	unsigned short HeadChecksum;	//ͷ��У���

	unsigned int SourceAddr;		//ԴIP��ַ

	unsigned int DestinAddr;		//Ŀ��IP��ַ

}ip_head;

/*
����У���
*/
unsigned short checksum(unsigned short * buffer, int size)
{
	unsigned long sum = 0;
	while (size > 1) {
		sum = sum + (*buffer);
		buffer++;
		size = size - sizeof(unsigned short);
	}
	if (size)
		sum = sum + *(unsigned char *)buffer;
	while ((sum >> 16) != 0) {
		sum = (sum >> 16) + (sum & 0xffff);
	}
	return (unsigned short)(~sum);
}


void main() {

	unsigned short * data = new unsigned short[2];
	data[0] = 0x0008;
	data[1] = (unsigned short) 7256;
	
	unsigned short c = checksum(data, 4);
	
	cout  << c<<endl;

}