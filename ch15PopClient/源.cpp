#include <conio.h>
#include <string>
#include <WinSock2.h>
#include <iostream>
#include "Դ.h"

using namespace std;

//Ԥ����ָ��,����ws2_32.lib�����
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
		cout << endl << "Socket Sendʧ�ܣ�" << endl;
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
		cout << endl << "Socket Recvʧ�ܣ�" << endl;
		closesocket(sock);
		return false;
	}
	if (memcmp(Respond,"+OK",3)==0)//�ɹ�
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
		cout << endl << "�밴���¸�ʽ���������У�PopClient server_addr" << endl;
		return;
	}

	WSADATA WSAData;
	//Winsock 2.2��ʼ��
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
	{
		cout << endl << "WSAStartup��ʼ��ʧ�ܣ�" << endl;
		return;
	}
	//�������������׽���
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		cout << endl << "����Socketʧ�ܣ�" << endl;
		return;
	}

	int ipaddress = inet_addr(argv[1]);//ip��ַ
	if (ipaddress == INADDR_NONE)//���������ַ�������һ���Ϸ���IP��ַ
	{
		hostent * pHostent = gethostbyname(argv[1]);
		if (pHostent)//���ǿ�ָ��
		{
			//�õ�IP��ַ
			ipaddress = (* (in_addr*)pHostent->h_addr).s_addr;
		}
	}

	sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(110);
	serveraddr.sin_addr.S_un.S_addr = ipaddress;

	//��POP3������������������
	int nConnect = connect(sock, (sockaddr*)&serveraddr, sizeof(serveraddr));
	if (nConnect == SOCKET_ERROR)
	{
		cout << endl << "Socket Connectʧ�ܣ�" << endl;
		return;
	}

	//�������Ӧ����Ϣ
	if (!RecvReply()) {
		//���ʧ��
		return;
	}
	else
	{
		if (RespondStatus==true)
		{//�����+OK
			cout << endl << Respond;
		}
		else
		{
			cout << endl << "Ӧ��״̬����";			
			return;
		}
	}

	//��POP3����������USER����
	cout << "POP3>USER:";
	memset(CmdBuf, 0, MAX_SIZE);
	cin.get(CmdBuf, MAX_SIZE,'\n');//�����û���
	ConstructCommand("USER ", CmdBuf);
	//�����������Ӧ��
	CommandSendAndRecv(retflag);
	if (retflag) return;
	

	//����PASS����
	cout << "POP3>PASS:";
	memset(CmdBuf, 0, MAX_SIZE);
	cout.flush();
	//��������
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
	//����PASS����
	CommandSendAndRecv(retflag);
	if (retflag) return;

	//�����������STAT����
	cout << "POP3>STAT"<<endl;
	//����STAT����
	ConstructCommand("STAT","");	
	//����STAT�������Ӧ��
	CommandSendAndRecv(retflag);
	if (retflag) return;
	cout.flush();

	cin.clear();
	cin.ignore(1024, '\n');//��ջ�����
	
	//��POP3����������LIST����
	cout << "POP3>LIST:";
	memset(CmdBuf, 0, MAX_SIZE);	
	cin.get(CmdBuf, MAX_SIZE,'\n');

	ConstructCommand("LIST ", CmdBuf);
	//����LIST�������Ӧ��
	CommandSendAndRecv(retflag);
	if (retflag) return;

	//��POP3����������QUIT����
	cout << "POP3>QUIT"<<endl;
	ConstructCommand("QUIT", "");
	//����QUIT�������Ӧ��
	CommandSendAndRecv(retflag);
	if (retflag) return;

	closesocket(sock);
	WSACleanup();
}

void CommandSendAndRecv(bool &retflag)
{
	retflag = false;
	//��������
	if (!SendCommand())
	{
		retflag = true;
		return;
	}

	//��������Ӧ����Ϣ
	if (!RecvReply()) {
		//���ʧ��
		retflag = true;
		return;
	}


	if (RespondStatus == true)
		cout<<Respond;
	else
	{
		cout << "Ӧ��״̬����" <<endl;
		cout << Respond;
		retflag = true;
		return;
	}
	retflag = false;
}

//����FTP���command����������para�ǲ���
void ConstructCommand(const char * command, const char *para)
{
	memset(Command, 0, MAX_SIZE);
	memcpy(Command, command, strlen(command));
	memcpy(Command + strlen(command), para, strlen(para));
	memcpy(Command + strlen(command) + strlen(para), "\r\n", 2);
}