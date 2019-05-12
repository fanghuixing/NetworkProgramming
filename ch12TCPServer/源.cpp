#include <string>
#include <WinSock2.h>
#include <iostream>

using namespace std;

#pragma comment(lib, "ws2_32")

struct ThreadParam
{
	SOCKET sock;
	sockaddr_in addr;
};

long ThreadCount = 0;
long *aa = &ThreadCount;

DWORD WINAPI ServerThread(LPVOID lpParam)
{
	//线程数目加1
	InterlockedIncrement(aa);
	cout << endl << "Server与Client建立连接";

	SOCKET tempsock = ((ThreadParam *) lpParam) ->sock;
	sockaddr_in tempaddr = ((ThreadParam*)lpParam)->addr;

	char recvbuf[65535];//接收缓冲区
	memset(recvbuf, 0, sizeof(recvbuf));
	int nRecv = recv(tempsock, recvbuf, sizeof(recvbuf), 0);
	if (nRecv==SOCKET_ERROR)
	{
		cout << endl << "Socket Recv失败！" << endl;
		closesocket(tempsock);//关闭临时Socket
		return 0;
	}

	if (strcmp(recvbuf, "sendfile")!=0)
	{//如果不是收到send file命令
		cout << endl << "没有收到sendfile命令" << endl;
		closesocket(tempsock);//关闭临时Socket
		return 0;
	}
	
	int nSend;
	//发送命令已收到信息给客户端
	nSend = send(tempsock, "command ok", sizeof("command ok"), 0);
	if (nSend == SOCKET_ERROR)
	{
		cout << endl << "Socket Send失败！" << endl;
		closesocket(tempsock);//关闭临时Socket
		return 0;
	}
	//接收客户端上传数据
	int size = sizeof(recvbuf);
	memset(recvbuf, 0, sizeof(recvbuf));
	nRecv = recv(tempsock, recvbuf, sizeof(recvbuf), 0);
	if (nRecv == SOCKET_ERROR)
	{
		cout << endl << "Socket Recv失败！" << endl;
		closesocket(tempsock);//关闭临时Socket
		return 0;
	}

	cout << endl << "Server接收数据：" << recvbuf << endl;

	closesocket(tempsock);//关闭临时Socket
	InterlockedDecrement(aa);
	cout << endl << "Server接收数据完成" << endl;
	return 0;
}


void main(int argc, char* argv[])
{
	if (argc!=2)
	{
		cout << endl << "请按以下格式输入命令行：TcpServer server_port" << endl;
		return;
	}

	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData))
	{
		cout << endl << "WSAStartup初始化失败" << endl;
		return;
	}
	//创建流式套接字
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		cout << endl << "创建Socket失败！" << endl;
		return;
	}

	sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;//IPv4
	serveraddr.sin_port = htons((unsigned short)atoi(argv[1]));
	serveraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//0.0.0.0

	int nBind;
	//套接字绑定IP地址和端口
	nBind = bind(sock, (sockaddr*)&serveraddr, sizeof(serveraddr));
	if (nBind == SOCKET_ERROR)
	{
		cout << endl << "Socket Bind失败！" << endl;
		return;
	}
	//侦听连接
	int nListen = listen(sock, SOMAXCONN);

	if (nListen == SOCKET_ERROR)
	{
		cout << endl << "Socket Listen失败！" << endl;
		return;
	}

	cout << endl << "Server开始侦听" << atoi(argv[1]) << "端口" << endl;

	SOCKET tempsock;
	sockaddr_in tempaddr;

	while (true)
	{

		int templen = sizeof(tempaddr);
		//从处于 established 状态的连接队列头部
		//取出一个已经完成的连接，如果这个队列没有
		//已经完成的连接，accept()函数就会阻塞，
		//直到取出队列中已完成的用户连接为止。
		tempsock = accept(
			sock, 
			(sockaddr*)&tempaddr, 
			&templen);
		if (tempsock==INVALID_SOCKET)
		{
			cout << endl << "Socket Accept失败！" << endl;
			return;
		}

		if (ThreadCount>=10)
		{//如果线程数达到上限10
			closesocket(tempsock);//关闭临时Socket
			continue;
		}

		ThreadParam Param;
		Param.sock = tempsock;
		Param.addr = tempaddr;
		DWORD dwThreadId; 
		//创建服务线程
		CreateThread(NULL, 0, ServerThread, &Param, 0, &dwThreadId);
	}
	closesocket(sock);
	WSACleanup();
}