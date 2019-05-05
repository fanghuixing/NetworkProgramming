#include <time.h>
#include <WinSock2.h>
#include <iostream>

using namespace std;
#pragma comment (lib, "ws2_32") //���߱��������������

#define ICMP_ECHO 8			//�������
#define ICMP_ECHO_REPLY 0	//�����Ӧ
#define ICMP_MIN 8			//��СICMP������
#define DEF_PACKET 32		//Ĭ�����ݳ���
#define MAX_PACKET 1024		//������󳤶�



//IPͷ���ṹ
typedef struct IP_HEAD
{
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

}ip_head;

//ICMPͷ���ṹ
typedef  struct ICMP_Head
{
	unsigned char Type;//��������(8λ)
	unsigned char Code;//����(8λ)
	unsigned short HeadChecksum;//У���(16λ)
	unsigned short Identifier;//��ʶ��(16λ)
	unsigned short Sequence;//���к�(16λ)
}icmp_head;



/*
����У���
*/
unsigned short checksum(unsigned short * buffer, int size)
{
	unsigned long sum = 0;
	while (size > 1) {
		sum = sum + (*buffer);
		buffer++;
		size = size - sizeof(unsigned short);
	}
	if (size)
		sum = sum + *(unsigned char *)buffer;
	while ((sum >> 16) != 0) {
		sum = (sum >> 16) + (sum & 0xffff);
	}
	return (unsigned short)(~sum);
}


void main(int argc, char * argv[])
{

	//��������в���
	if (argc != 2)
	{
		cout << endl << "�밴���¸�ʽ���������У�ScanHost host_addr" << endl;
		return;
	}


	WSADATA WSAData;
	//��2.2���winsock
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
	{
		cout << endl << "WSAStart��ʼ��ʧ�ܣ�" << endl;
		return;
	}

	//����ԭʼ�׽���
	SOCKET sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sock == INVALID_SOCKET)
	{
		cout << endl << "����Socketʧ�ܣ�" << endl;
		return;
	}

	int send_timeout = 1000; //���÷��ͳ�ʱ1000ms
	if (setsockopt(sock,
		SOL_SOCKET,
		SO_SNDTIMEO,
		(char *)&send_timeout,
		sizeof(send_timeout))
		== SOCKET_ERROR)
	{
		cout << endl << "���÷��ͳ�ʱʧ�ܣ�" << endl;
		return;
	}

	int recv_timeout = 1000;
	if (setsockopt(sock,
		SOL_SOCKET,
		SO_RCVTIMEO,
		(char *)&recv_timeout,
		sizeof(recv_timeout))
		== SOCKET_ERROR)
	{
		cout << endl << "���ý��ճ�ʱʧ�ܣ�" << endl;
		return;
	}

	//Ŀ�ĵ�ַ
	sockaddr_in dest;
	memset(&dest, 0, sizeof(dest));
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = inet_addr(argv[1]);//IP��ַ

	cout << endl << "��ʼ����ɨ��";

	//���ICMP���ݰ�
	char icmp_data[MAX_PACKET]; //�������
	memset(icmp_data, 0, MAX_PACKET);

	icmp_head * icmp_hdr;  //icmpͷ��ָ��
	int icmpsize = DEF_PACKET + sizeof(icmp_head); //ICMP����

	icmp_hdr = (icmp_head *)icmp_data;
	icmp_hdr->Type = ICMP_ECHO;   //���û������ͣ�8λ��
	icmp_hdr->Identifier = (unsigned short)GetCurrentThreadId();//�߳�ID����ʶ��
	icmp_hdr->HeadChecksum = 0;
	icmp_hdr->HeadChecksum = checksum((unsigned short *)icmp_data, icmpsize);
	
	//����ICMP���ݰ�
	int nSend = sendto(sock,
		icmp_data,
		icmpsize,
		0,
		(struct sockaddr *) &dest,
		sizeof(dest));

	if (nSend == SOCKET_ERROR || nSend < icmpsize)
	{
		cout << endl << "ICMP������ʧ�ܣ�" << endl;
		return;
	}

	sockaddr_in from;  //Դ��ַ
	int fromlen = sizeof(from);

	memset(&from, 0, sizeof(from));
	//���ջ�������IP���ݰ�
	char * recvbuf = new char[MAX_PACKET + sizeof(ip_head)];

    //����ICMP���ݰ�
	int nRecv = recvfrom(sock,
		recvbuf,
		MAX_PACKET + sizeof(ip_head),
		0,
		(struct sockaddr *) &from,
		&fromlen);

	if (nRecv == SOCKET_ERROR)
	{
		cout << endl << "����" << inet_ntoa(dest.sin_addr) << ":�ر�״̬" << endl;
		int error = WSAGetLastError();
		return;
	}

	ip_head * iphdr = (ip_head*)recvbuf;
	icmp_head * icmphdr;
	unsigned short ip_size = (iphdr->HeadLen & 0x0f) * 4; //IPͷ���ֽ���
	icmphdr = (icmp_head *)(recvbuf + ip_size); //����IP��ͷ��

	//�Խ��յ�ICMP�������ж�

	bool icmptype = true; //�Ƿ�����ȷ��Ӧ��

	//�Ƿ�������С����Ҫ��
	if (nRecv < ip_size + ICMP_MIN)
	{
		cout << endl << "���հ�̫�̣�������" << endl;
		icmpsize = false;
	}

	//�Ƿ��ǻ���Ӧ��
	if (icmphdr->Type != ICMP_ECHO_REPLY)
	{
		cout << endl << "���ǻ�����Ӧ��������" << endl;
		icmpsize = false;
	}

	//��ʶ���Ƿ�ƥ��
	if (icmphdr-> Identifier != (unsigned short) GetCurrentThreadId())
	{
		cout << endl << "ID�������������" << endl;
		icmpsize = false;
	}

	if (icmptype == true)
	{
		cout << endl << "����" << inet_ntoa(dest.sin_addr) << ":�״̬" << endl;
	}
	closesocket(sock);
	WSACleanup();
}
