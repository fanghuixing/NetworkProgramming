#include <string.h>
#include <time.h>
#include <WinSock2.h>
#include <fstream>
#include <iostream>

using namespace std;

#pragma comment(lib,"ws2_32")

void main(int argc, char * argv[])
{
	if (argc!=2)
	{
		cout << endl << "请按以下格式输入命令行：UdpServer server_port"
			<< endl;
		return;
	}

	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2,2), &WSAData)!=0)
	{
		cout << endl << "WSAStartup初始化失败" << endl;
		return;
	}

	SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock==INVALID_SOCKET)
	{
		cout << endl << "创建Socket失败！" << endl;
		return;
	}

	
	//填充本地套接字地址
	sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons((unsigned short)atoi(argv[1]));
	serveraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	int serveraddrlen = sizeof(serveraddr);
	
	//将套接字与套接字地址绑定
	int nBind = bind(sock, (sockaddr*)&serveraddr, sizeof(serveraddr));
	if (nBind==SOCKET_ERROR)
	{
		cout << endl << "Socket Bind失败！" << endl;
		return;
	}

	sockaddr_in clientaddr;
	int clientaddrlen = sizeof(clientaddr);

	while (true)
	{
		char recvbuf[20];//接收缓冲区
		memset(recvbuf, 0, sizeof(recvbuf));
		cout << endl << "等待客户端发送getfile或gettime命令" << endl;
		int nRecv;
		//接收命令
		nRecv = recvfrom(sock,
			recvbuf,
			sizeof(recvbuf),
			0,
			(sockaddr*)&clientaddr,
			&clientaddrlen);

		if (nRecv == SOCKET_ERROR)
		{
			cout << endl << "Socket Recv失败！" << endl;
			return;
		}

		cout << endl << "UDP Server接收命令：" << recvbuf << endl;

		//设置发送缓冲区
		char sendbuf[1500];
		memset(sendbuf, 0, sizeof(sendbuf));

		if (strcmp(recvbuf, "getfile") == 0)
		{
			//将文件数据写入发送缓冲区
			fstream infile;
			infile.open("input", ios::in);
			infile.seekg(0, ios::end);
			int nlength = infile.tellg();
			infile.seekg(0, ios::beg);
			infile.read(sendbuf, nlength);
			//向客户机发送数据
			int nsend;
			nsend = sendto(sock,
				sendbuf,          //保存待传数据的缓冲区
				sizeof(sendbuf),      //待传字节数
				0,             //可选项参数，若没有则传递0
				(sockaddr*)&clientaddr,       //目标地址
				clientaddrlen);

			if (nsend == SOCKET_ERROR)
			{
				cout << endl << "Socket Send失败！" << endl;
				return;
			}
		}
		else if (strcmp(recvbuf, "gettime") == 0)//gettime命令
		{
			//将系统时间写入发送缓冲区
			time_t   CurTime;
			time(&CurTime);
			struct tm t;
			localtime_s(&t, &CurTime);
			strftime(sendbuf,
				sizeof(sendbuf),
				"%Y-%m-%d %H:%M:%S", &t);
			//向客户机发送数据
			int nsend = sendto(sock,
				sendbuf,
				sizeof(sendbuf),
				0,
				(sockaddr*)&clientaddr,
				clientaddrlen);

			if (nsend == SOCKET_ERROR)
			{
				cout << endl << "Socket Send失败！" << endl;
				return;
			}

		}
		else {
			continue;
		}

		cout << "Server发送数据：" << sendbuf << endl;
		cout << endl << "等待客户端发送command ok命令" << endl;
		nRecv = recvfrom(sock,
			recvbuf,
			sizeof(recvbuf),
			0,
			(sockaddr*)&clientaddr,
			&clientaddrlen);

		if (nRecv == SOCKET_ERROR)
		{
			cout << endl << "Socket Recv失败！" << endl;
			return;
		}

		if (strcmp(recvbuf, "command ok") != 0)
			return;
	}

	cout << endl << "Server发送数据完成";
	closesocket(sock);
	WSACleanup();
	
}
