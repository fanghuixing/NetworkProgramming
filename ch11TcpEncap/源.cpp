#include <winsock2.h>
#include <fstream>
#include <iostream>

using namespace std;

#pragma comment(lib, "ws2_32")

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

//定义TCP伪头部结构
typedef struct PSD_HEAD
{
	unsigned int SourceAddr;		//源IPv4地址
	unsigned int DestinAddr;		//目的IPv4地址
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

unsigned short check[65535];//校验缓冲区
const char tcp_data[] = {"This is a test of tcp packet encapsule!"};

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

void main(int argc, char* argv[])
{
	if (argc != 2)
	{
		cout << endl << "请按以下格式输入命令行：TcpEncap output_file" << endl;
		return;
	}

	fstream outfile;
	outfile.open(argv[1], ios::in | ios::out | ios::trunc | ios::binary);

	//填充IP包头部各字段
	ip_head ip = { 0 };
	ip.Version = (0x04 << 4 | sizeof(ip_head) / sizeof(unsigned int));
	ip.ServiceType = 0;
	ip.TotalLen = htons( sizeof(ip_head) + sizeof(tcp_head) + sizeof(tcp_data));
	ip.Identifier = 0;
	ip.Protocol = IPPROTO_TCP;
	ip.HeadChecksum = 0;
	ip.SourceAddr = inet_addr("192.168.1.15");
	ip.DestinAddr = inet_addr("192.168.1.22");

	//计算IP包头部各字段的校验和
	memset(check, 0, 65535);
	memcpy(check, &ip, sizeof(ip_head));
	ip.HeadChecksum = checksum(check, sizeof(ip_head));


	//显示IP头部的部分字段值
	cout << endl << "IP头部字段" << endl;
	cout << "总长度：" << ntohs(ip.TotalLen) << endl;
	cout << "IP校验和：" << ip.HeadChecksum << endl;
	cout << "源IP地址：" << inet_ntoa(*(in_addr *)&ip.SourceAddr) << endl;
	cout << "目的IP地址：" << inet_ntoa(*(in_addr *)&ip.DestinAddr) << endl;

	//填充TCP伪头部各字段
	psd_head psd = { 0 };
	psd.SourceAddr = ip.SourceAddr;
	psd.DestinAddr = ip.DestinAddr;
	psd.Reserved = 0;
	psd.Protocol = ip.Protocol;
	psd.TcpLen = htons(sizeof(tcp_head) + sizeof(tcp_data));


	//填充TCP头部各字段
	tcp_head tcp = { 0 };
	tcp.SourcePort = htons(1000);
	tcp.DestinPort = htons(1000);
	tcp.Sequence = 0;
	tcp.Acknowledge = 0;
	tcp.HeadLen = htons( (sizeof(tcp_head) / sizeof(unsigned int) << 4 | 0));
	tcp.WindowsLen = htons((unsigned short)10000);
	tcp.TcpChecksum = 0;
	tcp.UrgePoint = 0;

	//计算TCP包（包括伪头部与数据）的校验和
	memset(check, 0, 65535);//全部重置为0
	memcpy(check, &psd, sizeof(psd_head));//复制伪头部
	memcpy(check + sizeof(psd_head), &tcp, sizeof(tcp_head));//复制tcp头部
	memcpy(check + sizeof(psd_head) + sizeof(tcp_head),	tcp_data, sizeof(tcp_data));//复制tcp数据
    //计算校验和
	tcp.TcpChecksum = checksum(check, sizeof(psd_head) + sizeof(tcp_head) + sizeof(tcp_data));
	
	//显示TCP头部的部分字段与数据部分
	cout << endl << "TCP头部与数据字段" << endl;
	cout <<  "TCP长度：" << ntohs(psd.TcpLen) << endl;
	cout << "源端口：" << ntohs(tcp.SourcePort) << endl;
	cout << "目的端口：" << ntohs(tcp.DestinPort) << endl;
	cout << "TCP校验和：" << tcp.TcpChecksum << endl;
	cout << "数据字段：" << tcp_data << endl;

	//依次写入IP头部、伪头部、TCP头部与数据
	outfile.write((char*)&ip, sizeof(ip_head));
	outfile.write((char*)&tcp, sizeof(tcp_head));
	outfile.write(tcp_data, sizeof(tcp_data));
	cout << endl << "TCP包封装完成" << endl;
	outfile.close();
}