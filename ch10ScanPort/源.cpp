#include <WinSock2.h>
#include <iostream>


using std::cout;
using std::endl;
#pragma comment(lib, "ws2_32")

void main(int argc, char * argv[])
{
	if (argc != 2)
	{
		cout << endl << "请按以下格式输入命令行：ScanPort server_addr" << endl;
		return;
	}

	WSADATA WSAData;
	//初始化Winsock DLL，成功返回0
	if (WSAStartup(MAKEWORD(2,2), &WSAData) != 0)
	{
		cout << endl << "WSAStartup初始化失败" << endl;
		return;
	}

	cout << endl << "已开启的TCP端口：";
	
	for (int i = 0; i < 128; i++)
	{
		//创建原始套接字
		SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock==INVALID_SOCKET)
		{
			cout << endl << "创建Socket失败！" << endl;
			return;
		}
		else
		{
			sockaddr_in serveraddr;
			serveraddr.sin_family = AF_INET;
			serveraddr.sin_port = htons((unsigned short)i);
			serveraddr.sin_addr.S_un.S_addr = inet_addr(argv[1]);

			int nConnect = connect(sock,
							(sockaddr *)&serveraddr,
							sizeof(serveraddr));

			if (nConnect==SOCKET_ERROR)//连接出错
			{
				continue;
			}

			//“可写”socket集合
			struct fd_set write;
			FD_ZERO(&write);//初始化为空集
			FD_SET(sock, &write);//将sock加入集合

			struct timeval timeout;//设置超时时间
			timeout.tv_sec = 100 / 1000;
			timeout.tv_usec = 0;
			if (select(0, NULL, &write, NULL, &timeout)>0)
			{//可写性意味着连接顺利建立，端口打开
				cout << i << " ";
			}
			//关闭原始套接字
			closesocket(sock);					   		
		}
	}

	//停止使用Winsock 2 DLL，释放资源
	WSACleanup();
	cout << endl << "TCP端口扫描完成" << endl;
}
