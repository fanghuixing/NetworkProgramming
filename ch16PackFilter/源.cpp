#include <winsock2.h>
#include <iostream>
#include <ws2tcpip.h>
using std::cout;
using std::endl;
using std::hex;
#pragma comment(lib, "ws2_32")
#define IO_RCVALL _WSAIOW(IOC_VENDOR, 1)
#define PERMIT_OPT 0  //定义过滤操作
#define REJECT_OPT 1



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

typedef struct
{
	char SourceAddr[16];//源IP地址
	char DestinAddr[16];//目的IP地址
	unsigned short SourcePort; //源端口号
	unsigned short DestinPort; //目的端口号
	unsigned char Protocol; //协议类型
	bool Operation; //操作类型
}filter_table;




void main(int argc, char * argv[])
{
	if (argc != 2)
	{
		cout << endl << "请按以下格式输入命令行：PackFilter packet_sum" << endl;
		return;
	}

	WSADATA WSAData;
	//初始化Winsock DLL
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
	{
		cout << endl << "WSAStartup初始化失败" << endl;
		return;
	}

	//创建原始套接字
	SOCKET sock = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
	if (sock == INVALID_SOCKET)
	{
		cout << endl << "创建Socket失败！" << endl;
		return;
	}



	
	BOOL flag = true;
///设置IP头操作选项
	if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, (char *)&flag, sizeof(flag)) == SOCKET_ERROR)
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
	if ((pHostIP = gethostbyname(hostName)) == NULL)
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
	if (bind(sock, (PSOCKADDR)&host_addr, sizeof(host_addr)) == SOCKET_ERROR)
	{
		cout << endl << "bind操作失败" << endl;
		return;
	}

	DWORD dwBufferLen[10];
	DWORD dwBufferInLen = 1;
	DWORD dwBytesReturned = 0;

	//设置网卡为混杂模式
	if (WSAIoctl(sock, IO_RCVALL, &dwBufferInLen, sizeof(dwBufferInLen),
		&dwBufferLen, sizeof(dwBufferLen), &dwBytesReturned, NULL, NULL) == SOCKET_ERROR)
	{
		cout << endl << "WSAIoctl操作失败" << endl;
		return;
	}

	//------------------填写包过滤规则（2项）---------------------
	filter_table filter[2];

	//第一条
	memset(filter[0].SourceAddr, 0, 16);//16个字节均用0进行填充
	//设置IP地址
	memcpy(filter[0].SourceAddr, "192.168.0.1", strlen("192.168.0.1"));
	//设置协议类型 
	filter[0].Protocol = IPPROTO_UDP;
	filter[0].Operation = REJECT_OPT;

	//第二条
	memset(filter[1].DestinAddr, 0, 16);//16个字节均用0进行填充
	//设置IP地址
	memcpy(filter[1].DestinAddr, "192.168.0.1", strlen("192.168.0.1"));
	//设置协议类型 
	filter[1].Protocol = IPPROTO_UDP;
	filter[1].Operation = PERMIT_OPT;



	cout << endl << "开始过滤IP包:" << endl;

	char buffer[65535]; //缓冲区
	int packsum = atoi(argv[1]);
	int i = 0;
	while (i < packsum)
	{
		memset(buffer, 0, 65535);
		if (recv(sock, buffer, 65535, 0) > 0)
		{
			ip_head ip = *(ip_head*)buffer;

			//获取源IP地址
			int addrlen = strlen(inet_ntoa(*(in_addr*)&ip.SourceAddr));
			char source_ip[16];
			memset(source_ip, 0, 16);
			//从ip头部中取出源ip地址存入source_ip
			memcpy(source_ip, inet_ntoa(*(in_addr*)&ip.SourceAddr), addrlen);
			
			//获取目的IP地址
			addrlen = strlen(inet_ntoa(*(in_addr*)&ip.DestinAddr));
			char destin_ip[16];
			memset(destin_ip, 0, 16);
			//从ip头部中取出目的ip地址存入destin_ip
			memcpy(destin_ip, inet_ntoa(*(in_addr*)&ip.DestinAddr), addrlen);					

			//检验包过滤规则1
			//比较IP包中的源IP地址与规则中的源IP地址
			if (strcmp(source_ip, filter[0].SourceAddr) == 0)
			{//源IP地址相同
				if (ip.Protocol == filter[0].Protocol)
				{//协议相同
					cout << "-------------------" << endl;
					cout << "源IP地址：" << inet_ntoa(*(in_addr*)&ip.SourceAddr) << endl;
					cout << "目的IP地址：" << inet_ntoa(*(in_addr*)&ip.DestinAddr) << endl;
					cout << "协议类型:UDP" << endl;
					cout << "操作类型：拒绝" << endl;
					i++;
				}

			}


			//检验包过滤规则2
			//比较IP包中的目的IP地址与规则中的目的IP地址
			if (strcmp(destin_ip, filter[1].DestinAddr) == 0)
			{//目的IP地址相同
				if (ip.Protocol == filter[1].Protocol)
				{//协议相同
					cout << "-------------------" << endl;
					cout << "源IP地址：" << inet_ntoa(*(in_addr*)&ip.SourceAddr) << endl;
					cout << "目的IP地址：" << inet_ntoa(*(in_addr*)&ip.DestinAddr) << endl;
					cout << "协议类型:UDP" << endl;
					cout << "操作类型：允许" << endl;
					i++;
				}
			}
		}
	}
	//关闭原始套接字
	closesocket(sock);
	WSACleanup();//卸载Winsock DLL，释放资源
}

