#include <iostream>
using namespace std;

//IP头部结构
typedef struct IP_HEAD
{
	union
	{
		unsigned char Version; //版本(字节前4位)
		unsigned char HeadLen; //头部长度(字节后4位)
	};

	unsigned char ServiceType;		//服务类型

	unsigned short TotalLen;		//总长度

	unsigned short Identifier;		//标识符

	union
	{
		unsigned short Flags;		//标志位(字前3位)
		unsigned short FragOffset;	//片偏移(字后13位)
	};

	unsigned char TimeToLive;		//生存周期

	unsigned char Protocol;			//协议

	unsigned short HeadChecksum;	//头部校验和

	unsigned int SourceAddr;		//源IP地址

	unsigned int DestinAddr;		//目的IP地址

}ip_head;

/*
计算校验和
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