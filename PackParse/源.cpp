#include <winsock2.h>
#include <iostream>
#include <ws2tcpip.h>
using std::cout;
using std::endl;

#pragma comment(lib, "ws2_32")
#define IO_RCVALL _WSAIOW(IOC_VENDOR, 1)


/*IP包头部结构*/
typedef struct IP_HEAD {
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

	unsigned char Options;			//选项
}ip_head;


void main(int argc, char * argv[])
{
	if (argc!=2)
	{
		cout << endl << "请按以下格式输入命令行：PackParse packet_sum" << endl;
		return;
	}

	WSADATA WSAData;
	//初始化Winsock DLL
	if (WSAStartup(MAKEWORD(2,2), &WSAData)!=0)
	{
		cout << endl << "WSAStartup初始化失败" << endl;
		return;
	}

	//创建原始套接字
	SOCKET sock = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
	if (sock==INVALID_SOCKET)
	{
		cout << endl << "创建Socket失败！" << endl;
		return;
	}

	BOOL flag = true;
	//设置IP头操作选项
	if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, (char *) &flag, sizeof(flag))==SOCKET_ERROR)
	{
		cout << endl << "setcockopt操作失败" << endl;
		return;
	}

	char hostName[128];
	//获得本地主机名
	if (gethostname(hostName, 100) == SOCKET_ERROR)
	{
		cout << endl << "gethostname操作失败" << endl;
		return;
	}

	hostent* pHostIP;
	//获取本地IP地址
	if ((pHostIP=gethostbyname(hostName)) == NULL)
	{
		cout << endl << "gethostbyname操作失败" << endl;
		return;
	}

	//填充sockaddr_in
	sockaddr_in host_addr;
	host_addr.sin_addr = *(in_addr *)pHostIP->h_addr_list[0];
	host_addr.sin_family = AF_INET;
	host_addr.sin_port = htons(6000);

	

	//绑定本地网卡
	if (bind(sock, (PSOCKADDR) &host_addr, sizeof(host_addr)) == SOCKET_ERROR)
	{
		cout << endl << "bind操作失败" << endl;
		return;
	}

	DWORD dwBufferLen[10];
	DWORD dwBufferInLen = 1;
	DWORD dwBytesReturned = 0;
	
	//设置网卡为混杂模式
	if (WSAIoctl(sock, IO_RCVALL, &dwBufferInLen, sizeof(dwBufferInLen), 
		&dwBufferLen, sizeof(dwBufferLen), &dwBytesReturned, NULL, NULL)==SOCKET_ERROR)
	{
		cout << endl << "WSAIoctl操作失败" << endl;
		return;
	}
	
	cout << endl << "开始解析IP包:" << endl;

	char buffer[65535]; //缓冲区
	int packsum = atoi(argv[1]);

	for (int i = 0; i < packsum; i++)
	{
		if (recv(sock, buffer, 65535, 0) > 0)
		{
			ip_head ip = *(ip_head*)buffer;
			cout << "-------------------------------------------------" << endl;
			cout << "版本：" << (ip.Version >> 4) << endl;
			cout << "头部长度：" << ((ip.HeadLen & 0x0f) * 4) << endl;
			cout << "服务器类型：Priority " << (ip.ServiceType >> 5)
				<< ", Service " << ((ip.ServiceType >> 1) & 0x0f) << endl;
			cout << "总长度：" << ntohs(ip.TotalLen) << endl;
			cout << "标识符：" << ntohs(ip.Identifier) << endl;
			ip.Flags = ntohs(ip.Flags);
			cout << "标志位：" << ((ip.Flags >> 15) & 0x01) << ", DF="
				<< ((ip.Flags >> 14) & 0x01) << ", MF=" << ((ip.Flags >> 13) & 0x01)
				<< endl;
			cout << "片偏移：" << (ip.FragOffset & 0x1fff) << endl;
			cout << "生存周期：" << (int)ip.TimeToLive << endl;
			cout << "协议：Protocol " << (int)ip.Protocol << endl;
			cout << "头部校验和：" << ntohs(ip.HeadChecksum) << endl;
			cout << "源IP地址：" << inet_ntoa( * (in_addr *) &ip.SourceAddr  ) << endl;
			cout << "目的IP地址：" << inet_ntoa(*(in_addr *)&ip.DestinAddr) << endl;
		}
	}
	//关闭原始套接字
	closesocket(sock);
	WSACleanup();//写作Winsock DLL
}
