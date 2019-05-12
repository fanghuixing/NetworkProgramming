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
	//�߳���Ŀ��1
	InterlockedIncrement(aa);
	cout << endl << "Server��Client��������";

	SOCKET tempsock = ((ThreadParam *) lpParam) ->sock;
	sockaddr_in tempaddr = ((ThreadParam*)lpParam)->addr;

	char recvbuf[65535];//���ջ�����
	memset(recvbuf, 0, sizeof(recvbuf));
	int nRecv = recv(tempsock, recvbuf, sizeof(recvbuf), 0);
	if (nRecv==SOCKET_ERROR)
	{
		cout << endl << "Socket Recvʧ�ܣ�" << endl;
		closesocket(tempsock);//�ر���ʱSocket
		return 0;
	}

	if (strcmp(recvbuf, "sendfile")!=0)
	{//��������յ�send file����
		cout << endl << "û���յ�sendfile����" << endl;
		closesocket(tempsock);//�ر���ʱSocket
		return 0;
	}
	
	int nSend;
	//�����������յ���Ϣ���ͻ���
	nSend = send(tempsock, "command ok", sizeof("command ok"), 0);
	if (nSend == SOCKET_ERROR)
	{
		cout << endl << "Socket Sendʧ�ܣ�" << endl;
		closesocket(tempsock);//�ر���ʱSocket
		return 0;
	}
	//���տͻ����ϴ�����
	int size = sizeof(recvbuf);
	memset(recvbuf, 0, sizeof(recvbuf));
	nRecv = recv(tempsock, recvbuf, sizeof(recvbuf), 0);
	if (nRecv == SOCKET_ERROR)
	{
		cout << endl << "Socket Recvʧ�ܣ�" << endl;
		closesocket(tempsock);//�ر���ʱSocket
		return 0;
	}

	cout << endl << "Server�������ݣ�" << recvbuf << endl;

	closesocket(tempsock);//�ر���ʱSocket
	InterlockedDecrement(aa);
	cout << endl << "Server�����������" << endl;
	return 0;
}


void main(int argc, char* argv[])
{
	if (argc!=2)
	{
		cout << endl << "�밴���¸�ʽ���������У�TcpServer server_port" << endl;
		return;
	}

	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData))
	{
		cout << endl << "WSAStartup��ʼ��ʧ��" << endl;
		return;
	}
	//������ʽ�׽���
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		cout << endl << "����Socketʧ�ܣ�" << endl;
		return;
	}

	sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;//IPv4
	serveraddr.sin_port = htons((unsigned short)atoi(argv[1]));
	serveraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//0.0.0.0

	int nBind;
	//�׽��ְ�IP��ַ�Ͷ˿�
	nBind = bind(sock, (sockaddr*)&serveraddr, sizeof(serveraddr));
	if (nBind == SOCKET_ERROR)
	{
		cout << endl << "Socket Bindʧ�ܣ�" << endl;
		return;
	}
	//��������
	int nListen = listen(sock, SOMAXCONN);

	if (nListen == SOCKET_ERROR)
	{
		cout << endl << "Socket Listenʧ�ܣ�" << endl;
		return;
	}

	cout << endl << "Server��ʼ����" << atoi(argv[1]) << "�˿�" << endl;

	SOCKET tempsock;
	sockaddr_in tempaddr;

	while (true)
	{

		int templen = sizeof(tempaddr);
		//�Ӵ��� established ״̬�����Ӷ���ͷ��
		//ȡ��һ���Ѿ���ɵ����ӣ�����������û��
		//�Ѿ���ɵ����ӣ�accept()�����ͻ�������
		//ֱ��ȡ������������ɵ��û�����Ϊֹ��
		tempsock = accept(
			sock, 
			(sockaddr*)&tempaddr, 
			&templen);
		if (tempsock==INVALID_SOCKET)
		{
			cout << endl << "Socket Acceptʧ�ܣ�" << endl;
			return;
		}

		if (ThreadCount>=10)
		{//����߳����ﵽ����10
			closesocket(tempsock);//�ر���ʱSocket
			continue;
		}

		ThreadParam Param;
		Param.sock = tempsock;
		Param.addr = tempaddr;
		DWORD dwThreadId; 
		//���������߳�
		CreateThread(NULL, 0, ServerThread, &Param, 0, &dwThreadId);
	}
	closesocket(sock);
	WSACleanup();
}