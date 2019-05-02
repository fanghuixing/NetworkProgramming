#include <string>
#include <WinSock2.h>
#include <iostream>
#include <fstream>
#include "Դ.h"

using  std::ios;
using  std::cout;
using  std::hex;
using  std::endl;
using  std::fstream;
#pragma comment(lib, "ws2_32")

typedef struct IP_HEAD
{
	union
	{
		unsigned int Version; //�汾(ǰ4λ)
		unsigned int Priority; //���ȼ�(��8λ)
		unsigned int FlowLabel; //�����(��20λ)
	};

	unsigned short PayloadLen;	//��Ч�غɳ���(16λ)
	unsigned char NextHead;	//��һ��ͷ��(8λ)
	unsigned char HopLimit; //��������(8λ)

	//Դ��ַ
	struct
	{
		__int64 Prefix;			//ǰ׺+�����Ų��֣�64λ��
		unsigned char MacAddr[8]; //ͨ��MAC��ַת���Ľӿڱ�ʶ���֣�64λ��
	}SourceAddr;

	//Ŀ�ĵ�ַ
	struct
	{
		__int64 Prefix;			//ǰ׺+�����Ų��֣�64λ��
		unsigned char MacAddr[8]; //ͨ��MAC��ַת���Ľӿڱ�ʶ���֣�64λ��
	}DestinAddr;

}ip_head;

//����TCPαͷ���ṹ
typedef struct PSD_HEAD
{
	unsigned char SourceAddr[16];		//ԴIP��ַ
	unsigned char DestinAddr[16];		//Ŀ��IP��ַ
	unsigned char Reserved;				//����λ
	unsigned char Protocol;				//Э��	
	unsigned short TcpLen;				//TCP����
}psd_head;


//����TCPͷ���ṹ
typedef struct TCP_HEAD
{
	unsigned short SourcePort;	//Դ�˿�
	unsigned short DestinPort;	//Ŀ�Ķ˿�
	unsigned int Sequence;		//���к�
	unsigned int Acknowledge;	//ȷ�Ϻ�
	union
	{
		unsigned short HeadLen; //ͷ������(ǰ4λ)
		unsigned short Reserved;	//����λ(��6λ) 
		unsigned short Flags;	//��־λ(��6λ)
	};
	unsigned short WindowsLen;	//���ڴ�С
	unsigned short TcpChecksum; //TCPУ���
	unsigned short UrgePoint;		//����ָ��
}tcp_head;

unsigned short check[65535]; //У�黺����
const char tcp_data[] = { "This is a test of ipv6 packet encapsule!" };

/*
����У��ͣ�16λ��
*/
unsigned short checksum(unsigned short * buffer, int size)
{
	unsigned long sum = 0;
	while (size > 0) {
		sum = sum + (*buffer);
		buffer++;
		size = size - sizeof(unsigned short);
	}
	while ((sum >> 16) != 0) {
		sum = (sum >> 16) + (sum & 0xffff);
	}
	return (unsigned short)(~sum);
}

/*
��64λ�������ֽ���ת�������ֽ���
*/
__int64 hton64(__int64 host64)
{
	char temp;
	//ͨ���ַ�ָ�룬ʵ���ֽڵ�λ�Ĵ���
	char * p = (char *)& host64; 
	for (int i = 0; i < 4; i++)
	{
		//�ֽ�ǰ�����
		temp = p[i];
		p[i] = p[7 - i];
		p[7 - i] = temp;
	}
	return host64;
}

void showIpAddr(std::fstream &outfile)
{
	
	for (int i = 0; i < 16; i++)
	{
	    
		cout << hex << outfile.get();
		if (i != 15)
		{
			cout << ":";
		}
	}
}

void main(int argc, char * argv[])
{
	//��������в���
	if (argc != 2)
	{
		cout << endl << "�밴���¸�ʽ���������У�"
			<< "ch8Ipv6Encap output_file" << endl;
		return;
	}

	fstream outfile; //�����ļ���
	outfile.open(argv[1], ios::out | ios::in | ios::trunc); //���ļ�

	//���IP��ͷ���ֶ�
	ip_head ip = { 0 };
	ip.Version = 6; //4λ�汾��6
	ip.Version <<= 8;	//�汾����8λ��Ҫ�����ȼ�
	ip.Version += 0;  //���8λ���ȼ���ȫ0
	ip.Version <<= 20; //���ȼ�����20λ��Ҫ�������
	ip.Version += 0; //���20λ�����
	ip.Version = htonl(ip.Version);
	//16λ��Ч�غɣ�������ͷ������ܳ���
	ip.PayloadLen = htons(sizeof(tcp_head) + sizeof(tcp_data));
	ip.NextHead = IPPROTO_TCP; //8λ�� ��һ��ͷ��
	ip.HopLimit = 128; //8λ����������


	//���ԴIP��ַ
	//���3λ��ַǰ׺
	ip.SourceAddr.Prefix = 0x01;
	//���45λ·��ǰ׺
	ip.SourceAddr.Prefix <<= 45;
	ip.SourceAddr.Prefix += 0x01;
	//���16λ������
	ip.SourceAddr.Prefix <<= 16;
	ip.SourceAddr.Prefix += 0x01;
	ip.SourceAddr.Prefix = hton64(ip.SourceAddr.Prefix);
	//MAC��ַת�ɽӿڱ�ʶ
	ip.SourceAddr.MacAddr[0] = char(0x02);
	ip.SourceAddr.MacAddr[1] = char(0x00);
	ip.SourceAddr.MacAddr[2] = char(0x80);
	ip.SourceAddr.MacAddr[3] = char(0xFF);
	ip.SourceAddr.MacAddr[4] = char(0xFE);
	ip.SourceAddr.MacAddr[5] = char(0x18);
	ip.SourceAddr.MacAddr[6] = char(0x6E);
	ip.SourceAddr.MacAddr[7] = char(0xE5);

	//���Ŀ��IP��ַ
	//���3λ��ַǰ׺
	ip.DestinAddr.Prefix = 0x01;
	//���45λ·��ǰ׺
	ip.DestinAddr.Prefix <<= 45;
	ip.DestinAddr.Prefix += 0x02;
	//���16λ������
	ip.DestinAddr.Prefix <<= 16;
	ip.DestinAddr.Prefix += 0x02;
	ip.DestinAddr.Prefix = hton64(ip.DestinAddr.Prefix);
	//MAC��ַת�ɽӿڱ�ʶ
	ip.DestinAddr.MacAddr[0] = char(0x02);
	ip.DestinAddr.MacAddr[1] = char(0x00);
	ip.DestinAddr.MacAddr[2] = char(0xE4);
	ip.DestinAddr.MacAddr[3] = char(0xFF);
	ip.DestinAddr.MacAddr[4] = char(0xFE);
	ip.DestinAddr.MacAddr[5] = char(0x86);
	ip.DestinAddr.MacAddr[6] = char(0x3A);
	ip.DestinAddr.MacAddr[7] = char(0xDC);

	//���TCPαͷ�����ֶ�
	psd_head psd = { 0 };
	//��Դ�ڴ��ַ����ʼλ��(&(ip.SourceAddr))��ʼ����16���ֽ�
	//��Ŀ���ڴ��ַ��(psd.SourceAddr)
	//����ԴIPv6��ַ
	memcpy(psd.SourceAddr, &(ip.SourceAddr), 16);
	//����Ŀ��IPv6��ַ
	memcpy(psd.DestinAddr, &(ip.DestinAddr), 16);
	psd.Reserved = 0;
	psd.Protocol = ip.NextHead;
	psd.TcpLen = sizeof(tcp_head) + sizeof(tcp_data);

	//���TCPͷ���ֶ�
	tcp_head tcp = { 0 };
	tcp.SourcePort = htons(1000);
	tcp.DestinPort = htons(1000);
	tcp.Sequence = 0;
	tcp.Acknowledge = 0;
	tcp.HeadLen = (sizeof(tcp_head) / sizeof(unsigned int) << 4 | 0);
	tcp.WindowsLen = htons((unsigned short) 10000);
	tcp.TcpChecksum = 0;
	tcp.UrgePoint = 0;

	//����TCP��(����αͷ��������)У���
	//��check������������ȫ������Ϊ0
	memset(check, 0, 65535);
	//׼�������������ݣ�tcpαͷ��+tcpͷ��+tcp����
	memcpy(check, &psd, sizeof(psd_head));
	memcpy(check + sizeof(psd_head), &tcp, sizeof(tcp_head));
	memcpy(check + sizeof(psd_head) + sizeof(tcp_head), tcp_data, sizeof(tcp_data));

	tcp.TcpChecksum = checksum(check, sizeof(psd_head) + sizeof(tcp_head) + sizeof(tcp_data));
	tcp.TcpChecksum = htons(tcp.TcpChecksum);
	
	//����д��IPͷ��TCPͷ������
	outfile.seekp(0);
	outfile.write((char *) &ip, sizeof(ip_head));
	outfile.write((char *) &tcp, sizeof(tcp_head));
	outfile.write(tcp_data, sizeof(tcp_data));
	outfile.seekg(8, ios::beg); //��λ��ԴIPv6��ַλ��

	//��ʾIPͷ���Ĳ����ֶ������ݲ���
	cout << endl << "IPv6ͷ���������ֶ�";
	cout << endl << "�汾��" << (ntohl(ip.Version) >> 28);
	cout << endl << "��Ч�غɳ��ȣ�" << ntohs(ip.PayloadLen);
	cout << endl << "��һ��ͷ����" << (int)ip.NextHead;
	cout << endl << "ԴIP��ַ��";
	
	showIpAddr(outfile);
	cout << endl << "Ŀ��IP��ַ��";
	showIpAddr(outfile);

	cout << endl << "�����ֶΣ�" << tcp_data << endl;
	cout << endl << "IPv6����װ���" << endl;
	outfile.close();
}