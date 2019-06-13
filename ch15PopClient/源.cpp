#include <conio.h>
#include <string>
#include <WinSock2.h>
#include <iostream>
#include "源.h"

using namespace std;

//预处理指令,链接ws2_32.lib这个库
#pragma comment(lib, "ws2_32")

#define MAX_SIZE 4096

char CmdBuf[MAX_SIZE];
char Command[MAX_SIZE];
char Respond[MAX_SIZE];
bool RespondStatus;

SOCKET sock;
bool retflag;

bool SendCommand()
{
	int nSend = send(sock, Command, strlen(Command), 0);
	if (nSend == SOCKET_ERROR)
	{
		cout << endl << "Socket Send失败！" << endl;
		return false;
	}
	return true;
}


bool RecvReply()
{
	int nRecv;
	memset(Respond, 0, MAX_SIZE);
	nRecv = recv(sock, Respond, MAX_SIZE, 0);

	if (nRecv == SOCKET_ERROR)
	{
		cout << endl << "Socket Recv失败！" << endl;
		closesocket(sock);
		return false;
	}
	if (memcmp(Respond,"+OK",3)==0)//成功
	{
		RespondStatus = true;
	}
	else {
		RespondStatus = false;
	}
	
	return true;
}

void main(int argc, char * argv[])
{
	if (argc != 2)
	{
		cout << endl << "请按以下格式输入命令行：PopClient server_addr" << endl;
		return;
	}

	WSADATA WSAData;
	//Winsock 2.2初始化
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
	{
		cout << endl << "WSAStartup初始化失败！" << endl;
		return;
	}
	//创建控制连接套接字
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		cout << endl << "创建Socket失败！" << endl;
		return;
	}

	int ipaddress = inet_addr(argv[1]);//ip地址
	if (ipaddress == INADDR_NONE)//如果传入的字符串不是一个合法的IP地址
	{
		hostent * pHostent = gethostbyname(argv[1]);
		if (pHostent)//不是空指针
		{
			//得到IP地址
			ipaddress = (* (in_addr*)pHostent->h_addr).s_addr;
		}
	}

	sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(110);
	serveraddr.sin_addr.S_un.S_addr = ipaddress;

	//向POP3服务器发送连接请求
	int nConnect = connect(sock, (sockaddr*)&serveraddr, sizeof(serveraddr));
	if (nConnect == SOCKET_ERROR)
	{
		cout << endl << "Socket Connect失败！" << endl;
		return;
	}

	//获得连接应答信息
	if (!RecvReply()) {
		//如果失败
		return;
	}
	else
	{
		if (RespondStatus==true)
		{//如果是+OK
			cout << endl << Respond;
		}
		else
		{
			cout << endl << "应答状态出错！";			
			return;
		}
	}

	//向POP3服务器发送USER命令
	cout << "POP3>USER:";
	memset(CmdBuf, 0, MAX_SIZE);
	cin.get(CmdBuf, MAX_SIZE,'\n');//输入用户名
	ConstructCommand("USER ", CmdBuf);
	//发送命令并接收应答
	CommandSendAndRecv(retflag);
	if (retflag) return;
	

	//发送PASS命令
	cout << "POP3>PASS:";
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
	CommandSendAndRecv(retflag);
	if (retflag) return;

	//向服务器发送STAT命令
	cout << "POP3>STAT"<<endl;
	//构造STAT命令
	ConstructCommand("STAT","");	
	//发送STAT命令并接收应答
	CommandSendAndRecv(retflag);
	if (retflag) return;
	cout.flush();

	cin.clear();
	cin.ignore(1024, '\n');//清空缓冲区
	
	//向POP3服务器发送LIST命令
	cout << "POP3>LIST:";
	memset(CmdBuf, 0, MAX_SIZE);	
	cin.get(CmdBuf, MAX_SIZE,'\n');

	ConstructCommand("LIST ", CmdBuf);
	//发送LIST命令并接收应答
	CommandSendAndRecv(retflag);
	if (retflag) return;

	//向POP3服务器发送QUIT命令
	cout << "POP3>QUIT"<<endl;
	ConstructCommand("QUIT", "");
	//发送QUIT命令并接收应答
	CommandSendAndRecv(retflag);
	if (retflag) return;

	closesocket(sock);
	WSACleanup();
}

void CommandSendAndRecv(bool &retflag)
{
	retflag = false;
	//发送命令
	if (!SendCommand())
	{
		retflag = true;
		return;
	}

	//获得命令的应答信息
	if (!RecvReply()) {
		//如果失败
		retflag = true;
		return;
	}


	if (RespondStatus == true)
		cout<<Respond;
	else
	{
		cout << "应答状态出错！" <<endl;
		cout << Respond;
		retflag = true;
		return;
	}
	retflag = false;
}

//构造FTP命令，command是命令名，para是参数
void ConstructCommand(const char * command, const char *para)
{
	memset(Command, 0, MAX_SIZE);
	memcpy(Command, command, strlen(command));
	memcpy(Command + strlen(command), para, strlen(para));
	memcpy(Command + strlen(command) + strlen(para), "\r\n", 2);
}