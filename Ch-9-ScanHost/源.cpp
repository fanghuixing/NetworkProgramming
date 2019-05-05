#include <time.h>
#include <WinSock2.h>
#include <iostream>

using namespace std;
#pragma comment (lib, "ws2_32") //告诉编译器链接这个库

#define ICMP_ECHO 8			//请求回送
#define ICMP_ECHO_REPLY 0	//请求回应
#define ICMP_MIN 8			//最小ICMP包长度
#define DEF_PACKET 32		//默认数据长度
#define MAX_PACKET 1024		//数据最大长度



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

//ICMP头部结构
typedef  struct ICMP_Head
{
	unsigned char Type;//基本类型(8位)
	unsigned char Code;//代码(8位)
	unsigned short HeadChecksum;//校验和(16位)
	unsigned short Identifier;//标识符(16位)
	unsigned short Sequence;//序列号(16位)
}icmp_head;



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


void main(int argc, char * argv[])
{

	//检查命令行参数
	if (argc != 2)
	{
		cout << endl << "请按以下格式输入命令行：ScanHost host_addr" << endl;
		return;
	}


	WSADATA WSAData;
	//绑定2.2版的winsock
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
	{
		cout << endl << "WSAStart初始化失败！" << endl;
		return;
	}

	//创建原始套接字
	SOCKET sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sock == INVALID_SOCKET)
	{
		cout << endl << "创建Socket失败！" << endl;
		return;
	}

	int send_timeout = 1000; //设置发送超时1000ms
	if (setsockopt(sock,
		SOL_SOCKET,
		SO_SNDTIMEO,
		(char *)&send_timeout,
		sizeof(send_timeout))
		== SOCKET_ERROR)
	{
		cout << endl << "设置发送超时失败！" << endl;
		return;
	}

	int recv_timeout = 1000;
	if (setsockopt(sock,
		SOL_SOCKET,
		SO_RCVTIMEO,
		(char *)&recv_timeout,
		sizeof(recv_timeout))
		== SOCKET_ERROR)
	{
		cout << endl << "设置接收超时失败！" << endl;
		return;
	}

	//目的地址
	sockaddr_in dest;
	memset(&dest, 0, sizeof(dest));
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = inet_addr(argv[1]);//IP地址

	cout << endl << "开始主机扫描";

	//填充ICMP数据包
	char icmp_data[MAX_PACKET]; //存放数据
	memset(icmp_data, 0, MAX_PACKET);

	icmp_head * icmp_hdr;  //icmp头部指针
	int icmpsize = DEF_PACKET + sizeof(icmp_head); //ICMP包长

	icmp_hdr = (icmp_head *)icmp_data;
	icmp_hdr->Type = ICMP_ECHO;   //设置基本类型（8位）
	icmp_hdr->Identifier = (unsigned short)GetCurrentThreadId();//线程ID做标识符
	icmp_hdr->HeadChecksum = 0;
	icmp_hdr->HeadChecksum = checksum((unsigned short *)icmp_data, icmpsize);
	
	//发送ICMP数据包
	int nSend = sendto(sock,
		icmp_data,
		icmpsize,
		0,
		(struct sockaddr *) &dest,
		sizeof(dest));

	if (nSend == SOCKET_ERROR || nSend < icmpsize)
	{
		cout << endl << "ICMP包发送失败！" << endl;
		return;
	}

	sockaddr_in from;  //源地址
	int fromlen = sizeof(from);

	memset(&from, 0, sizeof(from));
	//接收缓冲区，IP数据包
	char * recvbuf = new char[MAX_PACKET + sizeof(ip_head)];

    //接收ICMP数据包
	int nRecv = recvfrom(sock,
		recvbuf,
		MAX_PACKET + sizeof(ip_head),
		0,
		(struct sockaddr *) &from,
		&fromlen);

	if (nRecv == SOCKET_ERROR)
	{
		cout << endl << "主机" << inet_ntoa(dest.sin_addr) << ":关闭状态" << endl;
		int error = WSAGetLastError();
		return;
	}

	ip_head * iphdr = (ip_head*)recvbuf;
	icmp_head * icmphdr;
	unsigned short ip_size = (iphdr->HeadLen & 0x0f) * 4; //IP头部字节数
	icmphdr = (icmp_head *)(recvbuf + ip_size); //跳过IP包头部

	//对接收的ICMP包进行判断

	bool icmptype = true; //是否是正确的应答

	//是否满足最小长度要求
	if (nRecv < ip_size + ICMP_MIN)
	{
		cout << endl << "接收包太短，丢弃！" << endl;
		icmpsize = false;
	}

	//是否是回送应答
	if (icmphdr->Type != ICMP_ECHO_REPLY)
	{
		cout << endl << "不是回送响应，丢弃！" << endl;
		icmpsize = false;
	}

	//标识符是否匹配
	if (icmphdr-> Identifier != (unsigned short) GetCurrentThreadId())
	{
		cout << endl << "ID不相符，丢弃！" << endl;
		icmpsize = false;
	}

	if (icmptype == true)
	{
		cout << endl << "主机" << inet_ntoa(dest.sin_addr) << ":活动状态" << endl;
	}
	closesocket(sock);
	WSACleanup();
}
