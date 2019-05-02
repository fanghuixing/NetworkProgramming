#include <string>
#include <WinSock2.h>
#include <iostream>
#include <fstream>
#include "源.h"

using  std::ios;
using  std::cout;
using  std::hex;
using  std::endl;
using  std::fstream;
#pragma comment(lib, "ws2_32")

typedef struct IP_HEAD
{
	union
	{
		unsigned int Version; //版本(前4位)
		unsigned int Priority; //优先级(中8位)
		unsigned int FlowLabel; //流标号(后20位)
	};

	unsigned short PayloadLen;	//有效载荷长度(16位)
	unsigned char NextHead;	//下一个头部(8位)
	unsigned char HopLimit; //跳步限制(8位)

	//源地址
	struct
	{
		__int64 Prefix;			//前缀+子网号部分（64位）
		unsigned char MacAddr[8]; //通过MAC地址转换的接口标识部分（64位）
	}SourceAddr;

	//目的地址
	struct
	{
		__int64 Prefix;			//前缀+子网号部分（64位）
		unsigned char MacAddr[8]; //通过MAC地址转换的接口标识部分（64位）
	}DestinAddr;

}ip_head;

//定义TCP伪头部结构
typedef struct PSD_HEAD
{
	unsigned char SourceAddr[16];		//源IP地址
	unsigned char DestinAddr[16];		//目的IP地址
	unsigned char Reserved;				//保留位
	unsigned char Protocol;				//协议	
	unsigned short TcpLen;				//TCP长度
}psd_head;


//定义TCP头部结构
typedef struct TCP_HEAD
{
	unsigned short SourcePort;	//源端口
	unsigned short DestinPort;	//目的端口
	unsigned int Sequence;		//序列号
	unsigned int Acknowledge;	//确认号
	union
	{
		unsigned short HeadLen; //头部长度(前4位)
		unsigned short Reserved;	//保留位(中6位) 
		unsigned short Flags;	//标志位(后6位)
	};
	unsigned short WindowsLen;	//窗口大小
	unsigned short TcpChecksum; //TCP校验和
	unsigned short UrgePoint;		//紧急指针
}tcp_head;

unsigned short check[65535]; //校验缓冲区
const char tcp_data[] = { "This is a test of ipv6 packet encapsule!" };

/*
计算校验和（16位）
*/
unsigned short checksum(unsigned short * buffer, int size)
{
	unsigned long sum = 0;
	while (size > 0) {
		sum = sum + (*buffer);
		buffer++;
		size = size - sizeof(unsigned short);
	}
	while ((sum >> 16) != 0) {
		sum = (sum >> 16) + (sum & 0xffff);
	}
	return (unsigned short)(~sum);
}

/*
（64位）主机字节序转成网络字节序
*/
__int64 hton64(__int64 host64)
{
	char temp;
	//通过字符指针，实现字节单位的处理
	char * p = (char *)& host64; 
	for (int i = 0; i < 4; i++)
	{
		//字节前后调换
		temp = p[i];
		p[i] = p[7 - i];
		p[7 - i] = temp;
	}
	return host64;
}

void showIpAddr(std::fstream &outfile)
{
	
	for (int i = 0; i < 16; i++)
	{
	    
		cout << hex << outfile.get();
		if (i != 15)
		{
			cout << ":";
		}
	}
}

void main(int argc, char * argv[])
{
	//检查命令行参数
	if (argc != 2)
	{
		cout << endl << "请按以下格式输入命令行："
			<< "ch8Ipv6Encap output_file" << endl;
		return;
	}

	fstream outfile; //创建文件流
	outfile.open(argv[1], ios::out | ios::in | ios::trunc); //打开文件

	//填充IP包头部字段
	ip_head ip = { 0 };
	ip.Version = 6; //4位版本：6
	ip.Version <<= 8;	//版本后面8位需要放优先级
	ip.Version += 0;  //填充8位优先级，全0
	ip.Version <<= 20; //优先级后面20位需要放流标号
	ip.Version += 0; //填充20位流标号
	ip.Version = htonl(ip.Version);
	//16位有效载荷，除基本头部外的总长度
	ip.PayloadLen = htons(sizeof(tcp_head) + sizeof(tcp_data));
	ip.NextHead = IPPROTO_TCP; //8位， 下一个头部
	ip.HopLimit = 128; //8位，跳步限制


	//填充源IP地址
	//填充3位地址前缀
	ip.SourceAddr.Prefix = 0x01;
	//填充45位路由前缀
	ip.SourceAddr.Prefix <<= 45;
	ip.SourceAddr.Prefix += 0x01;
	//填充16位子网号
	ip.SourceAddr.Prefix <<= 16;
	ip.SourceAddr.Prefix += 0x01;
	ip.SourceAddr.Prefix = hton64(ip.SourceAddr.Prefix);
	//MAC地址转成接口标识
	ip.SourceAddr.MacAddr[0] = char(0x02);
	ip.SourceAddr.MacAddr[1] = char(0x00);
	ip.SourceAddr.MacAddr[2] = char(0x80);
	ip.SourceAddr.MacAddr[3] = char(0xFF);
	ip.SourceAddr.MacAddr[4] = char(0xFE);
	ip.SourceAddr.MacAddr[5] = char(0x18);
	ip.SourceAddr.MacAddr[6] = char(0x6E);
	ip.SourceAddr.MacAddr[7] = char(0xE5);

	//填充目的IP地址
	//填充3位地址前缀
	ip.DestinAddr.Prefix = 0x01;
	//填充45位路由前缀
	ip.DestinAddr.Prefix <<= 45;
	ip.DestinAddr.Prefix += 0x02;
	//填充16位子网号
	ip.DestinAddr.Prefix <<= 16;
	ip.DestinAddr.Prefix += 0x02;
	ip.DestinAddr.Prefix = hton64(ip.DestinAddr.Prefix);
	//MAC地址转成接口标识
	ip.DestinAddr.MacAddr[0] = char(0x02);
	ip.DestinAddr.MacAddr[1] = char(0x00);
	ip.DestinAddr.MacAddr[2] = char(0xE4);
	ip.DestinAddr.MacAddr[3] = char(0xFF);
	ip.DestinAddr.MacAddr[4] = char(0xFE);
	ip.DestinAddr.MacAddr[5] = char(0x86);
	ip.DestinAddr.MacAddr[6] = char(0x3A);
	ip.DestinAddr.MacAddr[7] = char(0xDC);

	//填充TCP伪头部各字段
	psd_head psd = { 0 };
	//从源内存地址的起始位置(&(ip.SourceAddr))开始拷贝16个字节
	//到目标内存地址中(psd.SourceAddr)
	//拷贝源IPv6地址
	memcpy(psd.SourceAddr, &(ip.SourceAddr), 16);
	//拷贝目的IPv6地址
	memcpy(psd.DestinAddr, &(ip.DestinAddr), 16);
	psd.Reserved = 0;
	psd.Protocol = ip.NextHead;
	psd.TcpLen = sizeof(tcp_head) + sizeof(tcp_data);

	//填充TCP头部字段
	tcp_head tcp = { 0 };
	tcp.SourcePort = htons(1000);
	tcp.DestinPort = htons(1000);
	tcp.Sequence = 0;
	tcp.Acknowledge = 0;
	tcp.HeadLen = (sizeof(tcp_head) / sizeof(unsigned int) << 4 | 0);
	tcp.WindowsLen = htons((unsigned short) 10000);
	tcp.TcpChecksum = 0;
	tcp.UrgePoint = 0;

	//计算TCP包(包括伪头部与数据)校验和
	//将check缓冲区的内容全部设置为0
	memset(check, 0, 65535);
	//准备计算所需数据：tcp伪头部+tcp头部+tcp数据
	memcpy(check, &psd, sizeof(psd_head));
	memcpy(check + sizeof(psd_head), &tcp, sizeof(tcp_head));
	memcpy(check + sizeof(psd_head) + sizeof(tcp_head), tcp_data, sizeof(tcp_data));

	tcp.TcpChecksum = checksum(check, sizeof(psd_head) + sizeof(tcp_head) + sizeof(tcp_data));
	tcp.TcpChecksum = htons(tcp.TcpChecksum);
	
	//依次写入IP头、TCP头与数据
	outfile.seekp(0);
	outfile.write((char *) &ip, sizeof(ip_head));
	outfile.write((char *) &tcp, sizeof(tcp_head));
	outfile.write(tcp_data, sizeof(tcp_data));
	outfile.seekg(8, ios::beg); //定位到源IPv6地址位置

	//显示IP头部的部分字段与数据部分
	cout << endl << "IPv6头部与数据字段";
	cout << endl << "版本：" << (ntohl(ip.Version) >> 28);
	cout << endl << "有效载荷长度：" << ntohs(ip.PayloadLen);
	cout << endl << "下一个头部：" << (int)ip.NextHead;
	cout << endl << "源IP地址：";
	
	showIpAddr(outfile);
	cout << endl << "目的IP地址：";
	showIpAddr(outfile);

	cout << endl << "数据字段：" << tcp_data << endl;
	cout << endl << "IPv6包封装完成" << endl;
	outfile.close();
}