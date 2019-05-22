#include <conio.h>
#include <WinSock2.h>
#include <iostream>
#include "Դ.h"

using namespace std;

#pragma comment(lib, "ws2_32")

#define MAX_SIZE 4096

char CmdBuf[MAX_SIZE];//���뻺����
char Command[MAX_SIZE];//�ͻ�������
char ReplyMsg[MAX_SIZE];//������Ӧ��
int nReplyCode; //������Ӧ����
bool bConnected = false;//�Ƿ��ѵ�¼
SOCKET SocketControl; //���������׽���
SOCKET SocketData; //���������׽���

//��FTP����������Ӧ��
bool RecvReply()
{
	int nRecv; 
	memset(ReplyMsg, 0, MAX_SIZE);
	nRecv = recv(SocketControl, ReplyMsg, MAX_SIZE, 0);

	if (nRecv==SOCKET_ERROR)
	{
		cout << endl << "Socket Recvʧ�ܣ�" << endl;
		closesocket(SocketControl);
		return false;
	}
	if (nRecv>4)
	{//�з�������
		char * ReplyCodes = new char[3];//Ӧ�����ַ���
		memset(ReplyCodes, 0, 3);
		memcpy(ReplyCodes, ReplyMsg, 3);
		nReplyCode = atoi(ReplyCodes);//Ӧ�����ַ���ת����
	}
	return true;
}

//��FTP��������������
bool SendCommand()
{
	int nSend = send(SocketControl, Command, strlen(Command), 0);
	if (nSend==SOCKET_ERROR)
	{
		cout << endl << "Socket Sendʧ�ܣ�" << endl;
		return false;
	}
	return true;	
}

//������������
bool DataConnect(char * ServerAddr)
{
	//����PASV����
	ConstructCommand("PASV", "");
	if (!SendCommand())//����PASV����
	{
		return false;
	}

	//���PASV�����Ӧ����Ϣ
	//���磺227 entering passive mode (127,0,0,1,4,18)
	if (RecvReply())
	{
		if (nReplyCode != 227)//����޷����뱻��ģʽ
		{
			cout << endl << "PASV����Ӧ�����" << endl;
			closesocket(SocketControl);
			return false;
		}
		else {
			cout << ReplyMsg;
		}
	}

	//����PASV�����Ӧ����Ϣ
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

	//��ȡFTP�����������ݶ˿�
	unsigned short ServerPort = 
		unsigned short((atoi(part[4])<<8) + atoi(part[5]));
	//cout <<"�������Ӷ˿ڣ�"<< ServerPort << endl;
	SocketData = socket(AF_INET, SOCK_STREAM, 0);
	if (SocketData == INVALID_SOCKET)
	{
		cout << endl << "����Socketʧ�ܣ�" << endl;
		return false;
	}

	sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(ServerPort);
	serveraddr.sin_addr.S_un.S_addr = inet_addr(ServerAddr);

	//��FTP������������������
	int nConnect;
	nConnect = connect(SocketData, (sockaddr*)&serveraddr, sizeof(serveraddr));
	if (nConnect == SOCKET_ERROR)
	{
		cout << endl << "Socket Connectʧ�ܣ�" << endl;
		return false;
	}
	return true;



}

void main(int argc, char *argv[])
{
	if (argc != 2)
	{
		cout << endl << "�밴���¸�ʽ���������У�FtpClient server_addr" << endl;
		return;
	}

	WSADATA WSAData;
	//Winsock 2.2��ʼ��
	if (WSAStartup(MAKEWORD(2,2), &WSAData)!=0)
	{
		cout << endl << "WSAStartup��ʼ��ʧ�ܣ�" << endl;
		return;
	}
	//�������������׽���
	SocketControl = socket(AF_INET, SOCK_STREAM, 0);
	if (SocketControl == INVALID_SOCKET)
	{
		cout << endl << "����Socketʧ�ܣ�" << endl;
		return;
	}

	//��������ַ
	sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(21);
	serveraddr.sin_addr.S_un.S_addr = inet_addr(argv[1]);

	//��FTP������������������
	cout << endl << "FTP>Control Connect...";
	int nConnect;
	nConnect = connect(SocketControl, 
		(sockaddr*)&serveraddr, 
		sizeof(serveraddr));

	if (nConnect == SOCKET_ERROR)
	{
		cout << endl << "Socket Connectʧ�ܣ�" << endl;
		return;
	}

	//�������Ӧ����Ϣ
	if (RecvReply())
	{
		if (nReplyCode == 220) //������׼������
			cout << endl << ReplyMsg;
		else
		{
			cout << endl << "ConnectӦ�����" << endl;
			closesocket(SocketControl);
			return;
		}
	}

	//����USER����
	cout << "FTP>USER:";
	memset(CmdBuf, 0, MAX_SIZE);
	cin.getline(CmdBuf, MAX_SIZE, '\n');//�Ӽ��̽��������ַ������Ի��з����жϽ���

	ConstructCommand("USER ", CmdBuf);
	//��������
	if (!SendCommand())
	{
		return;
	}

	//���USER�����Ӧ����Ϣ
	if (RecvReply())
	{
		if (nReplyCode==230 || nReplyCode==331)
		{//��¼�ɹ�������Ҫ����
			cout << ReplyMsg;
		}
		else {
			cout << ReplyMsg;
			cout << endl << "USER����Ӧ�����" << endl;
			closesocket(SocketControl);
			return;
		}
	}

	if (nReplyCode == 331) {
		//��FTP����������PASS����
		cout << "FTP>PASS:";
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
		if (!SendCommand())
		{
			return;
		}

		//���PASS�����Ӧ����Ϣ
		if (RecvReply())
		{
			if (nReplyCode == 230)//��¼�ɹ�
			{
				cout << ReplyMsg;
			}
			else
			{
				cout << endl << "PASS����Ӧ�����" << endl;
				closesocket(SocketControl);
				return;
			}
		}
	}

	//��FTP����������LIST����
	cout << "FTP>LIST" << endl;
	char FtpServer[MAX_SIZE];//������ip��ַ
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

	//���LIST����Ӧ����Ϣ
	if (RecvReply())
	{
		if (nReplyCode == 125 || nReplyCode == 150 || nReplyCode == 226)
			cout << ReplyMsg;
		else
		{
			cout << endl << "LIST����Ӧ�����" << endl;
			closesocket(SocketControl);
			return;
		}
	}

	//���Ŀ¼��Ϣ
	int nRecv;
	char ListBuf[MAX_SIZE];//���Ŀ¼��Ϣ
	while (true)
	{
		memset(ListBuf, 0, MAX_SIZE);
		nRecv = recv(SocketData, ListBuf, MAX_SIZE, 0);
		if (nRecv == SOCKET_ERROR)
		{
			cout << endl << "Socket Recvʧ�ܣ�" << endl;
			closesocket(SocketData);
			return;
		}

		if (nRecv<=0)
		{
			break;
		}
		//��ʾĿ¼��Ϣ
		cout << ListBuf;
	}
	//�ر���������
	closesocket(SocketData);
	
	//�������
	if (RecvReply())
	{
		if (nReplyCode == 226)//��������
			cout << ReplyMsg;
		else {
			cout << endl << "LIST����Ӧ�����" << endl;
			closesocket(SocketControl);
		}
	}

	//��FTP����������QUIT����
	cout << "FTP>QUIT" << endl;

	ConstructCommand("QUIT", "");
	if (!SendCommand())
	{
		return;
	}

	//���QUIT����Ӧ����Ϣ
	if (RecvReply())
	{
		if (nReplyCode==221)//����ر�
		{
			cout << ReplyMsg;
			bConnected = false;
			closesocket(SocketControl);
			return;
		}
		else
		{
			cout << endl << "QUIT����Ӧ�����" << endl;
			closesocket(SocketControl);
			return;
		}
	}
	WSACleanup();

}
//����FTP���command����������para�ǲ���
void ConstructCommand(const char * command, const char *para)
{
	memset(Command, 0, MAX_SIZE);
	memcpy(Command, command, strlen(command));
	memcpy(Command + strlen(command), para, strlen(para));
	memcpy(Command + strlen(command) + strlen(para), "\r\n", 2);
}
