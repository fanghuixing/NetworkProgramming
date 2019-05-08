#include <WinSock2.h>
#include <iostream>


using std::cout;
using std::endl;
#pragma comment(lib, "ws2_32")

void main(int argc, char * argv[])
{
	if (argc != 2)
	{
		cout << endl << "�밴���¸�ʽ���������У�ScanPort server_addr" << endl;
		return;
	}

	WSADATA WSAData;
	//��ʼ��Winsock DLL���ɹ�����0
	if (WSAStartup(MAKEWORD(2,2), &WSAData) != 0)
	{
		cout << endl << "WSAStartup��ʼ��ʧ��" << endl;
		return;
	}

	cout << endl << "�ѿ�����TCP�˿ڣ�";
	
	for (int i = 0; i < 128; i++)
	{
		//����ԭʼ�׽���
		SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock==INVALID_SOCKET)
		{
			cout << endl << "����Socketʧ�ܣ�" << endl;
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

			if (nConnect==SOCKET_ERROR)//���ӳ���
			{
				continue;
			}

			//����д��socket����
			struct fd_set write;
			FD_ZERO(&write);//��ʼ��Ϊ�ռ�
			FD_SET(sock, &write);//��sock���뼯��

			struct timeval timeout;//���ó�ʱʱ��
			timeout.tv_sec = 100 / 1000;
			timeout.tv_usec = 0;
			if (select(0, NULL, &write, NULL, &timeout)>0)
			{//��д����ζ������˳���������˿ڴ�
				cout << i << " ";
			}
			//�ر�ԭʼ�׽���
			closesocket(sock);					   		
		}
	}

	//ֹͣʹ��Winsock 2 DLL���ͷ���Դ
	WSACleanup();
	cout << endl << "TCP�˿�ɨ�����" << endl;
}
