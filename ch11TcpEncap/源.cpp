#include <winsock2.h>
#include <fstream>
#include <iostream>

using namespace std;

#pragma comment(lib, "ws2_32")

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

//����TCPαͷ���ṹ
typedef struct PSD_HEAD
{
	unsigned int SourceAddr;		//ԴIPv4��ַ
	unsigned int DestinAddr;		//Ŀ��IPv4��ַ
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

unsigned short check[65535];//У�黺����
const char tcp_data[] = {"This is a test of tcp packet encapsule!"};

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

void main(int argc, char* argv[])
{
	if (argc != 2)
	{
		cout << endl << "�밴���¸�ʽ���������У�TcpEncap output_file" << endl;
		return;
	}

	fstream outfile;
	outfile.open(argv[1], ios::in | ios::out | ios::trunc | ios::binary);

	//���IP��ͷ�����ֶ�
	ip_head ip = { 0 };
	ip.Version = (0x04 << 4 | sizeof(ip_head) / sizeof(unsigned int));
	ip.ServiceType = 0;
	ip.TotalLen = htons( sizeof(ip_head) + sizeof(tcp_head) + sizeof(tcp_data));
	ip.Identifier = 0;
	ip.Protocol = IPPROTO_TCP;
	ip.HeadChecksum = 0;
	ip.SourceAddr = inet_addr("192.168.1.15");
	ip.DestinAddr = inet_addr("192.168.1.22");

	//����IP��ͷ�����ֶε�У���
	memset(check, 0, 65535);
	memcpy(check, &ip, sizeof(ip_head));
	ip.HeadChecksum = checksum(check, sizeof(ip_head));


	//��ʾIPͷ���Ĳ����ֶ�ֵ
	cout << endl << "IPͷ���ֶ�" << endl;
	cout << "�ܳ��ȣ�" << ntohs(ip.TotalLen) << endl;
	cout << "IPУ��ͣ�" << ip.HeadChecksum << endl;
	cout << "ԴIP��ַ��" << inet_ntoa(*(in_addr *)&ip.SourceAddr) << endl;
	cout << "Ŀ��IP��ַ��" << inet_ntoa(*(in_addr *)&ip.DestinAddr) << endl;

	//���TCPαͷ�����ֶ�
	psd_head psd = { 0 };
	psd.SourceAddr = ip.SourceAddr;
	psd.DestinAddr = ip.DestinAddr;
	psd.Reserved = 0;
	psd.Protocol = ip.Protocol;
	psd.TcpLen = htons(sizeof(tcp_head) + sizeof(tcp_data));


	//���TCPͷ�����ֶ�
	tcp_head tcp = { 0 };
	tcp.SourcePort = htons(1000);
	tcp.DestinPort = htons(1000);
	tcp.Sequence = 0;
	tcp.Acknowledge = 0;
	tcp.HeadLen = htons( (sizeof(tcp_head) / sizeof(unsigned int) << 4 | 0));
	tcp.WindowsLen = htons((unsigned short)10000);
	tcp.TcpChecksum = 0;
	tcp.UrgePoint = 0;

	//����TCP��������αͷ�������ݣ���У���
	memset(check, 0, 65535);//ȫ������Ϊ0
	memcpy(check, &psd, sizeof(psd_head));//����αͷ��
	memcpy(check + sizeof(psd_head), &tcp, sizeof(tcp_head));//����tcpͷ��
	memcpy(check + sizeof(psd_head) + sizeof(tcp_head),	tcp_data, sizeof(tcp_data));//����tcp����
    //����У���
	tcp.TcpChecksum = checksum(check, sizeof(psd_head) + sizeof(tcp_head) + sizeof(tcp_data));
	
	//��ʾTCPͷ���Ĳ����ֶ������ݲ���
	cout << endl << "TCPͷ���������ֶ�" << endl;
	cout <<  "TCP���ȣ�" << ntohs(psd.TcpLen) << endl;
	cout << "Դ�˿ڣ�" << ntohs(tcp.SourcePort) << endl;
	cout << "Ŀ�Ķ˿ڣ�" << ntohs(tcp.DestinPort) << endl;
	cout << "TCPУ��ͣ�" << tcp.TcpChecksum << endl;
	cout << "�����ֶΣ�" << tcp_data << endl;

	//����д��IPͷ����αͷ����TCPͷ��������
	outfile.write((char*)&ip, sizeof(ip_head));
	outfile.write((char*)&tcp, sizeof(tcp_head));
	outfile.write(tcp_data, sizeof(tcp_data));
	cout << endl << "TCP����װ���" << endl;
	outfile.close();
}