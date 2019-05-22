#include <conio.h>
#include <WinSock2.h>
#include <iostream>
#include "源.h"

using namespace std;

#pragma comment(lib, "ws2_32")

#define MAX_SIZE 4096

char CmdBuf[MAX_SIZE];//输入缓冲区
char Command[MAX_SIZE];//客户机命令
char ReplyMsg[MAX_SIZE];//服务器应答
int nReplyCode; //服务器应答码
bool bConnected = false;//是否已登录
SOCKET SocketControl; //控制连接套接字
SOCKET SocketData; //数据连接套接字

//从FTP服务器接收应答
bool RecvReply()
{
	int nRecv; 
	memset(ReplyMsg, 0, MAX_SIZE);
	nRecv = recv(SocketControl, ReplyMsg, MAX_SIZE, 0);

	if (nRecv==SOCKET_ERROR)
	{
		cout << endl << "Socket Recv失败！" << endl;
		closesocket(SocketControl);
		return false;
	}
	if (nRecv>4)
	{//有返回数据
		char * ReplyCodes = new char[3];//应答码字符串
		memset(ReplyCodes, 0, 3);
		memcpy(ReplyCodes, ReplyMsg, 3);
		nReplyCode = atoi(ReplyCodes);//应答码字符串转整数
	}
	return true;
}

//向FTP服务器发送命令
bool SendCommand()
{
	int nSend = send(SocketControl, Command, strlen(Command), 0);
	if (nSend==SOCKET_ERROR)
	{
		cout << endl << "Socket Send失败！" << endl;
		return false;
	}
	return true;	
}

//建立数据连接
bool DataConnect(char * ServerAddr)
{
	//构造PASV命令
	ConstructCommand("PASV", "");
	if (!SendCommand())//发送PASV命令
	{
		return false;
	}

	//获得PASV命令的应答信息
	//比如：227 entering passive mode (127,0,0,1,4,18)
	if (RecvReply())
	{
		if (nReplyCode != 227)//如果无法进入被动模式
		{
			cout << endl << "PASV命令应答出错！" << endl;
			closesocket(SocketControl);
			return false;
		}
		else {
			cout << ReplyMsg;
		}
	}

	//分离PASV命令的应答信息
	char * part[6];
	
	if (strtok(ReplyMsg, "("))
	{
		for (int i = 0; i < 5; i++)
		{
			part[i] = strtok(NULL, ",");
			if (!part[i])
			{
				return false;
			}
		}

		part[5] = strtok(NULL, ")");
		if (!part[5])
		{
			return false;
		}
	}
	else return false;

	//获取FTP服务器的数据端口
	unsigned short ServerPort = 
		unsigned short((atoi(part[4])<<8) + atoi(part[5]));
	//cout <<"数据连接端口："<< ServerPort << endl;
	SocketData = socket(AF_INET, SOCK_STREAM, 0);
	if (SocketData == INVALID_SOCKET)
	{
		cout << endl << "创建Socket失败！" << endl;
		return false;
	}

	sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(ServerPort);
	serveraddr.sin_addr.S_un.S_addr = inet_addr(ServerAddr);

	//向FTP服务器发送连接请求
	int nConnect;
	nConnect = connect(SocketData, (sockaddr*)&serveraddr, sizeof(serveraddr));
	if (nConnect == SOCKET_ERROR)
	{
		cout << endl << "Socket Connect失败！" << endl;
		return false;
	}
	return true;



}

void main(int argc, char *argv[])
{
	if (argc != 2)
	{
		cout << endl << "请按以下格式输入命令行：FtpClient server_addr" << endl;
		return;
	}

	WSADATA WSAData;
	//Winsock 2.2初始化
	if (WSAStartup(MAKEWORD(2,2), &WSAData)!=0)
	{
		cout << endl << "WSAStartup初始化失败！" << endl;
		return;
	}
	//创建控制连接套接字
	SocketControl = socket(AF_INET, SOCK_STREAM, 0);
	if (SocketControl == INVALID_SOCKET)
	{
		cout << endl << "创建Socket失败！" << endl;
		return;
	}

	//服务器地址
	sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(21);
	serveraddr.sin_addr.S_un.S_addr = inet_addr(argv[1]);

	//向FTP服务器发送连接请求
	cout << endl << "FTP>Control Connect...";
	int nConnect;
	nConnect = connect(SocketControl, 
		(sockaddr*)&serveraddr, 
		sizeof(serveraddr));

	if (nConnect == SOCKET_ERROR)
	{
		cout << endl << "Socket Connect失败！" << endl;
		return;
	}

	//获得连接应答信息
	if (RecvReply())
	{
		if (nReplyCode == 220) //服务器准备好了
			cout << endl << ReplyMsg;
		else
		{
			cout << endl << "Connect应答出错！" << endl;
			closesocket(SocketControl);
			return;
		}
	}

	//发送USER命令
	cout << "FTP>USER:";
	memset(CmdBuf, 0, MAX_SIZE);
	cin.getline(CmdBuf, MAX_SIZE, '\n');//从键盘接收输入字符串，以换行符来判断结束

	ConstructCommand("USER ", CmdBuf);
	//发送命令
	if (!SendCommand())
	{
		return;
	}

	//获得USER命令的应答信息
	if (RecvReply())
	{
		if (nReplyCode==230 || nReplyCode==331)
		{//登录成功或者需要密码
			cout << ReplyMsg;
		}
		else {
			cout << ReplyMsg;
			cout << endl << "USER命令应答出错！" << endl;
			closesocket(SocketControl);
			return;
		}
	}

	if (nReplyCode == 331) {
		//向FTP服务器发送PASS命令
		cout << "FTP>PASS:";
		memset(CmdBuf, 0, MAX_SIZE);
		cout.flush();
		//输入密码
		for (int i = 0; i < MAX_SIZE; i++)
		{
			CmdBuf[i] = getch();
			if (CmdBuf[i] == '\r')
			{
				CmdBuf[i] = '\0';
				break;
			}
			else
				cout << '*';
		}

		cout << endl;

		ConstructCommand("PASS ", CmdBuf);
		//发送PASS命令
		if (!SendCommand())
		{
			return;
		}

		//获得PASS命令的应答信息
		if (RecvReply())
		{
			if (nReplyCode == 230)//登录成功
			{
				cout << ReplyMsg;
			}
			else
			{
				cout << endl << "PASS命令应答出错！" << endl;
				closesocket(SocketControl);
				return;
			}
		}
	}

	//向FTP服务器发送LIST命令
	cout << "FTP>LIST" << endl;
	char FtpServer[MAX_SIZE];//服务器ip地址
	memset(FtpServer, 0, MAX_SIZE);
	memcpy(FtpServer, argv[1], strlen(argv[1]));

	if (!DataConnect(FtpServer))
	{
		return;
	}

	ConstructCommand("LIST", "");
	if (!SendCommand())
	{
		return;
	}

	//获得LIST命令应答信息
	if (RecvReply())
	{
		if (nReplyCode == 125 || nReplyCode == 150 || nReplyCode == 226)
			cout << ReplyMsg;
		else
		{
			cout << endl << "LIST命令应答出错！" << endl;
			closesocket(SocketControl);
			return;
		}
	}

	//获得目录信息
	int nRecv;
	char ListBuf[MAX_SIZE];//存放目录信息
	while (true)
	{
		memset(ListBuf, 0, MAX_SIZE);
		nRecv = recv(SocketData, ListBuf, MAX_SIZE, 0);
		if (nRecv == SOCKET_ERROR)
		{
			cout << endl << "Socket Recv失败！" << endl;
			closesocket(SocketData);
			return;
		}

		if (nRecv<=0)
		{
			break;
		}
		//显示目录信息
		cout << ListBuf;
	}
	//关闭数据连接
	closesocket(SocketData);
	
	//传输结束
	if (RecvReply())
	{
		if (nReplyCode == 226)//结束传输
			cout << ReplyMsg;
		else {
			cout << endl << "LIST命令应答出错" << endl;
			closesocket(SocketControl);
		}
	}

	//向FTP服务器发送QUIT命令
	cout << "FTP>QUIT" << endl;

	ConstructCommand("QUIT", "");
	if (!SendCommand())
	{
		return;
	}

	//获得QUIT命令应答信息
	if (RecvReply())
	{
		if (nReplyCode==221)//服务关闭
		{
			cout << ReplyMsg;
			bConnected = false;
			closesocket(SocketControl);
			return;
		}
		else
		{
			cout << endl << "QUIT命令应答出错！" << endl;
			closesocket(SocketControl);
			return;
		}
	}
	WSACleanup();

}
//构造FTP命令，command是命令名，para是参数
void ConstructCommand(const char * command, const char *para)
{
	memset(Command, 0, MAX_SIZE);
	memcpy(Command, command, strlen(command));
	memcpy(Command + strlen(command), para, strlen(para));
	memcpy(Command + strlen(command) + strlen(para), "\r\n", 2);
}
