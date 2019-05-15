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
		cout << endl << "�밴���¸�ʽ���������У�UdpServer server_port"
			<< endl;
		return;
	}

	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2,2), &WSAData)!=0)
	{
		cout << endl << "WSAStartup��ʼ��ʧ��" << endl;
		return;
	}

	SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock==INVALID_SOCKET)
	{
		cout << endl << "����Socketʧ�ܣ�" << endl;
		return;
	}

	
	//��䱾���׽��ֵ�ַ
	sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons((unsigned short)atoi(argv[1]));
	serveraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	int serveraddrlen = sizeof(serveraddr);
	
	//���׽������׽��ֵ�ַ��
	int nBind = bind(sock, (sockaddr*)&serveraddr, sizeof(serveraddr));
	if (nBind==SOCKET_ERROR)
	{
		cout << endl << "Socket Bindʧ�ܣ�" << endl;
		return;
	}

	sockaddr_in clientaddr;
	int clientaddrlen = sizeof(clientaddr);

	while (true)
	{
		char recvbuf[20];//���ջ�����
		memset(recvbuf, 0, sizeof(recvbuf));
		cout << endl << "�ȴ��ͻ��˷���getfile��gettime����" << endl;
		int nRecv;
		//��������
		nRecv = recvfrom(sock,
			recvbuf,
			sizeof(recvbuf),
			0,
			(sockaddr*)&clientaddr,
			&clientaddrlen);

		if (nRecv == SOCKET_ERROR)
		{
			cout << endl << "Socket Recvʧ�ܣ�" << endl;
			return;
		}

		cout << endl << "UDP Server�������" << recvbuf << endl;

		//���÷��ͻ�����
		char sendbuf[1500];
		memset(sendbuf, 0, sizeof(sendbuf));

		if (strcmp(recvbuf, "getfile") == 0)
		{
			//���ļ�����д�뷢�ͻ�����
			fstream infile;
			infile.open("input", ios::in);
			infile.seekg(0, ios::end);
			int nlength = infile.tellg();
			infile.seekg(0, ios::beg);
			infile.read(sendbuf, nlength);
			//��ͻ�����������
			int nsend;
			nsend = sendto(sock,
				sendbuf,          //����������ݵĻ�����
				sizeof(sendbuf),      //�����ֽ���
				0,             //��ѡ���������û���򴫵�0
				(sockaddr*)&clientaddr,       //Ŀ���ַ
				clientaddrlen);

			if (nsend == SOCKET_ERROR)
			{
				cout << endl << "Socket Sendʧ�ܣ�" << endl;
				return;
			}
		}
		else if (strcmp(recvbuf, "gettime") == 0)//gettime����
		{
			//��ϵͳʱ��д�뷢�ͻ�����
			time_t   CurTime;
			time(&CurTime);
			struct tm t;
			localtime_s(&t, &CurTime);
			strftime(sendbuf,
				sizeof(sendbuf),
				"%Y-%m-%d %H:%M:%S", &t);
			//��ͻ�����������
			int nsend = sendto(sock,
				sendbuf,
				sizeof(sendbuf),
				0,
				(sockaddr*)&clientaddr,
				clientaddrlen);

			if (nsend == SOCKET_ERROR)
			{
				cout << endl << "Socket Sendʧ�ܣ�" << endl;
				return;
			}

		}
		else {
			continue;
		}

		cout << "Server�������ݣ�" << sendbuf << endl;
		cout << endl << "�ȴ��ͻ��˷���command ok����" << endl;
		nRecv = recvfrom(sock,
			recvbuf,
			sizeof(recvbuf),
			0,
			(sockaddr*)&clientaddr,
			&clientaddrlen);

		if (nRecv == SOCKET_ERROR)
		{
			cout << endl << "Socket Recvʧ�ܣ�" << endl;
			return;
		}

		if (strcmp(recvbuf, "command ok") != 0)
			return;
	}

	cout << endl << "Server�����������";
	closesocket(sock);
	WSACleanup();
	
}
