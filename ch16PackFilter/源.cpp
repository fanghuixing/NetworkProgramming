#include <winsock2.h>
#include <iostream>
#include <ws2tcpip.h>
using std::cout;
using std::endl;
using std::hex;
#pragma comment(lib, "ws2_32")
#define IO_RCVALL _WSAIOW(IOC_VENDOR, 1)
#define PERMIT_OPT 0  //������˲���
#define REJECT_OPT 1



/*IP��ͷ���ṹ*/
typedef struct IP_HEAD {
	union
	{
		unsigned char Version; //�汾(�ֽ�ǰ4λ)
		unsigned char HeadLen; //ͷ������(�ֽں�4λ)
	};

	unsigned char ServiceType;		//��������

	unsigned short TotalLen;		//�ܳ���

	unsigned short Identifier;		//��ʶ��

	union
	{
		unsigned short Flags;		//��־λ(��ǰ3λ)
		unsigned short FragOffset;	//Ƭƫ��(�ֺ�13λ)
	};

	unsigned char TimeToLive;		//��������

	unsigned char Protocol;			//Э��

	unsigned short HeadChecksum;	//ͷ��У���

	unsigned int SourceAddr;		//ԴIP��ַ

	unsigned int DestinAddr;		//Ŀ��IP��ַ

	unsigned char Options;			//ѡ��
}ip_head;

typedef struct
{
	char SourceAddr[16];//ԴIP��ַ
	char DestinAddr[16];//Ŀ��IP��ַ
	unsigned short SourcePort; //Դ�˿ں�
	unsigned short DestinPort; //Ŀ�Ķ˿ں�
	unsigned char Protocol; //Э������
	bool Operation; //��������
}filter_table;




void main(int argc, char * argv[])
{
	if (argc != 2)
	{
		cout << endl << "�밴���¸�ʽ���������У�PackFilter packet_sum" << endl;
		return;
	}

	WSADATA WSAData;
	//��ʼ��Winsock DLL
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
	{
		cout << endl << "WSAStartup��ʼ��ʧ��" << endl;
		return;
	}

	//����ԭʼ�׽���
	SOCKET sock = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
	if (sock == INVALID_SOCKET)
	{
		cout << endl << "����Socketʧ�ܣ�" << endl;
		return;
	}



	
	BOOL flag = true;
///����IPͷ����ѡ��
	if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, (char *)&flag, sizeof(flag)) == SOCKET_ERROR)
	{
		cout << endl << "setcockopt����ʧ��" << endl;
		return;
	}

	char hostName[128];
	//��ñ���������
	if (gethostname(hostName, 100) == SOCKET_ERROR)
	{
		cout << endl << "gethostname����ʧ��" << endl;
		return;
	}

	hostent* pHostIP;
	//��ȡ����IP��ַ
	if ((pHostIP = gethostbyname(hostName)) == NULL)
	{
		cout << endl << "gethostbyname����ʧ��" << endl;
		return;
	}

	//���sockaddr_in
	sockaddr_in host_addr;
	host_addr.sin_addr = *(in_addr *)pHostIP->h_addr_list[0];

	host_addr.sin_family = AF_INET;
	host_addr.sin_port = htons(6000);



	//�󶨱�������
	if (bind(sock, (PSOCKADDR)&host_addr, sizeof(host_addr)) == SOCKET_ERROR)
	{
		cout << endl << "bind����ʧ��" << endl;
		return;
	}

	DWORD dwBufferLen[10];
	DWORD dwBufferInLen = 1;
	DWORD dwBytesReturned = 0;

	//��������Ϊ����ģʽ
	if (WSAIoctl(sock, IO_RCVALL, &dwBufferInLen, sizeof(dwBufferInLen),
		&dwBufferLen, sizeof(dwBufferLen), &dwBytesReturned, NULL, NULL) == SOCKET_ERROR)
	{
		cout << endl << "WSAIoctl����ʧ��" << endl;
		return;
	}

	//------------------��д�����˹���2�---------------------
	filter_table filter[2];

	//��һ��
	memset(filter[0].SourceAddr, 0, 16);//16���ֽھ���0�������
	//����IP��ַ
	memcpy(filter[0].SourceAddr, "192.168.0.1", strlen("192.168.0.1"));
	//����Э������ 
	filter[0].Protocol = IPPROTO_UDP;
	filter[0].Operation = REJECT_OPT;

	//�ڶ���
	memset(filter[1].DestinAddr, 0, 16);//16���ֽھ���0�������
	//����IP��ַ
	memcpy(filter[1].DestinAddr, "192.168.0.1", strlen("192.168.0.1"));
	//����Э������ 
	filter[1].Protocol = IPPROTO_UDP;
	filter[1].Operation = PERMIT_OPT;



	cout << endl << "��ʼ����IP��:" << endl;

	char buffer[65535]; //������
	int packsum = atoi(argv[1]);
	int i = 0;
	while (i < packsum)
	{
		memset(buffer, 0, 65535);
		if (recv(sock, buffer, 65535, 0) > 0)
		{
			ip_head ip = *(ip_head*)buffer;

			//��ȡԴIP��ַ
			int addrlen = strlen(inet_ntoa(*(in_addr*)&ip.SourceAddr));
			char source_ip[16];
			memset(source_ip, 0, 16);
			//��ipͷ����ȡ��Դip��ַ����source_ip
			memcpy(source_ip, inet_ntoa(*(in_addr*)&ip.SourceAddr), addrlen);
			
			//��ȡĿ��IP��ַ
			addrlen = strlen(inet_ntoa(*(in_addr*)&ip.DestinAddr));
			char destin_ip[16];
			memset(destin_ip, 0, 16);
			//��ipͷ����ȡ��Ŀ��ip��ַ����destin_ip
			memcpy(destin_ip, inet_ntoa(*(in_addr*)&ip.DestinAddr), addrlen);					

			//��������˹���1
			//�Ƚ�IP���е�ԴIP��ַ������е�ԴIP��ַ
			if (strcmp(source_ip, filter[0].SourceAddr) == 0)
			{//ԴIP��ַ��ͬ
				if (ip.Protocol == filter[0].Protocol)
				{//Э����ͬ
					cout << "-------------------" << endl;
					cout << "ԴIP��ַ��" << inet_ntoa(*(in_addr*)&ip.SourceAddr) << endl;
					cout << "Ŀ��IP��ַ��" << inet_ntoa(*(in_addr*)&ip.DestinAddr) << endl;
					cout << "Э������:UDP" << endl;
					cout << "�������ͣ��ܾ�" << endl;
					i++;
				}

			}


			//��������˹���2
			//�Ƚ�IP���е�Ŀ��IP��ַ������е�Ŀ��IP��ַ
			if (strcmp(destin_ip, filter[1].DestinAddr) == 0)
			{//Ŀ��IP��ַ��ͬ
				if (ip.Protocol == filter[1].Protocol)
				{//Э����ͬ
					cout << "-------------------" << endl;
					cout << "ԴIP��ַ��" << inet_ntoa(*(in_addr*)&ip.SourceAddr) << endl;
					cout << "Ŀ��IP��ַ��" << inet_ntoa(*(in_addr*)&ip.DestinAddr) << endl;
					cout << "Э������:UDP" << endl;
					cout << "�������ͣ�����" << endl;
					i++;
				}
			}
		}
	}
	//�ر�ԭʼ�׽���
	closesocket(sock);
	WSACleanup();//ж��Winsock DLL���ͷ���Դ
}

