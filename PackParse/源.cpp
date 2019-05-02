#include <winsock2.h>
#include <iostream>
#include <ws2tcpip.h>
using std::cout;
using std::endl;

#pragma comment(lib, "ws2_32")
#define IO_RCVALL _WSAIOW(IOC_VENDOR, 1)


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


void main(int argc, char * argv[])
{
	if (argc!=2)
	{
		cout << endl << "�밴���¸�ʽ���������У�PackParse packet_sum" << endl;
		return;
	}

	WSADATA WSAData;
	//��ʼ��Winsock DLL
	if (WSAStartup(MAKEWORD(2,2), &WSAData)!=0)
	{
		cout << endl << "WSAStartup��ʼ��ʧ��" << endl;
		return;
	}

	//����ԭʼ�׽���
	SOCKET sock = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
	if (sock==INVALID_SOCKET)
	{
		cout << endl << "����Socketʧ�ܣ�" << endl;
		return;
	}

	BOOL flag = true;
	//����IPͷ����ѡ��
	if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, (char *) &flag, sizeof(flag))==SOCKET_ERROR)
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
	if ((pHostIP=gethostbyname(hostName)) == NULL)
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
	if (bind(sock, (PSOCKADDR) &host_addr, sizeof(host_addr)) == SOCKET_ERROR)
	{
		cout << endl << "bind����ʧ��" << endl;
		return;
	}

	DWORD dwBufferLen[10];
	DWORD dwBufferInLen = 1;
	DWORD dwBytesReturned = 0;
	
	//��������Ϊ����ģʽ
	if (WSAIoctl(sock, IO_RCVALL, &dwBufferInLen, sizeof(dwBufferInLen), 
		&dwBufferLen, sizeof(dwBufferLen), &dwBytesReturned, NULL, NULL)==SOCKET_ERROR)
	{
		cout << endl << "WSAIoctl����ʧ��" << endl;
		return;
	}
	
	cout << endl << "��ʼ����IP��:" << endl;

	char buffer[65535]; //������
	int packsum = atoi(argv[1]);

	for (int i = 0; i < packsum; i++)
	{
		if (recv(sock, buffer, 65535, 0) > 0)
		{
			ip_head ip = *(ip_head*)buffer;
			cout << "-------------------------------------------------" << endl;
			cout << "�汾��" << (ip.Version >> 4) << endl;
			cout << "ͷ�����ȣ�" << ((ip.HeadLen & 0x0f) * 4) << endl;
			cout << "���������ͣ�Priority " << (ip.ServiceType >> 5)
				<< ", Service " << ((ip.ServiceType >> 1) & 0x0f) << endl;
			cout << "�ܳ��ȣ�" << ntohs(ip.TotalLen) << endl;
			cout << "��ʶ����" << ntohs(ip.Identifier) << endl;
			ip.Flags = ntohs(ip.Flags);
			cout << "��־λ��" << ((ip.Flags >> 15) & 0x01) << ", DF="
				<< ((ip.Flags >> 14) & 0x01) << ", MF=" << ((ip.Flags >> 13) & 0x01)
				<< endl;
			cout << "Ƭƫ�ƣ�" << (ip.FragOffset & 0x1fff) << endl;
			cout << "�������ڣ�" << (int)ip.TimeToLive << endl;
			cout << "Э�飺Protocol " << (int)ip.Protocol << endl;
			cout << "ͷ��У��ͣ�" << ntohs(ip.HeadChecksum) << endl;
			cout << "ԴIP��ַ��" << inet_ntoa( * (in_addr *) &ip.SourceAddr  ) << endl;
			cout << "Ŀ��IP��ַ��" << inet_ntoa(*(in_addr *)&ip.DestinAddr) << endl;
		}
	}
	//�ر�ԭʼ�׽���
	closesocket(sock);
	WSACleanup();//д��Winsock DLL
}
