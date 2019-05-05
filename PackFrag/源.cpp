#include <string>
#include <WinSock2.h>
#include <fstream>
#include <iostream>

using std::endl;
using std::cout;
using std::fstream;
using std::ios;

#pragma comment (lib, "ws2_32")

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


/*
buffer : IPͷ���У��μ������16λ��ֵ
size�� �μ������IPͷ�����ֽ�����
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
	if (argc != 3)
	{
		cout << endl << "�밴���¸�ʽ���������У�PackFrag input_file output_file"
			<< endl;
		return;
	}

	//����ļ�
	fstream outfile;
	outfile.open(argv[2], ios::out);

	fstream infile;
	infile.open(argv[1], ios::in);

	if (!infile.is_open())
	{
		cout << endl << "�޷��������ļ�" << endl;
		return;
	}

	infile.seekg(0, ios::end); //�ļ�������ָ��ָ���β
	//��������ļ����ȣ�����Ƭ�����ݳ���һ��ʼ�����ļ�����
	int data_length = infile.tellg(); 
	infile.seekg(0, ios::beg);	//�ļ�ָ��ָ��ʼ

	bool is_more_frag = true; //�����ж��Ƿ������Ƭ
	int frag_data_length = 0;//��ʼ����Ƭ���ݳ���
	int frag_num = 0;//��ʼ����Ƭ���
	unsigned short check[10];//����У�黺������С
	while (is_more_frag)
	{
		ip_head ip = { 0 }; //��ʼ��ipͷ
		
		frag_num++; //��Ƭ��ż�1
		cout << endl << "IP��" << frag_num << "��ʼ��װ" << endl;
		//�������Ƭ����������80�ֽڣ���Ӧ���ǵ����һ����Ƭ�ˣ������и����Ƭ
		if (data_length < 80) 
		{
			is_more_frag = false;//�������һ����Ƭ��
			frag_data_length = data_length; //���õ�ǰ��Ƭ����
			//���ñ�־Ϊ000�����ֽڴ��ƫ����
			ip.Flags = unsigned short((frag_num-1)*10); //(80/8=10)
		}
		else {//�������һ����Ƭ
			data_length = data_length - 80; //�������ݳ��ȣ�ÿ����Ƭ�ֵ�80���ֽ�
			frag_data_length = 80; //��Ȼ��Ƭ����
			//���ñ�־Ϊ001�����ֽڴ��ƫ����
			ip.Flags = unsigned short( ((frag_num-1)*10) | 0x2000);
		}
		ip.Flags = htons(ip.Flags);

		//����IPͷ�������ֶ�

		//ǰ��λ������4����4λ����4�ֽ�Ϊ��λ��ipͷ����
		ip.Version = (0x04 << 4 | sizeof(ip_head) / sizeof(unsigned int));
		ip.ServiceType = unsigned char(0x00);
		//��Ƭ���ݳ���+IPͷ����
		ip.TotalLen = htons( unsigned short(frag_data_length + 20)); 
		ip.Identifier = htons(unsigned short(0x1000));
		ip.TimeToLive = unsigned char(0x80); //128
		ip.Protocol = unsigned char(0x06); //tcp
		ip.HeadChecksum = unsigned short(0x00); //��ʼУ�����0
		ip.SourceAddr = unsigned int(0x3801a8c0);
		ip.DestinAddr = unsigned int(0x3502a8c0);

		//��������check����20���ֽڵ�ֵ��Ϊֵ0��
		memset(check, 0, 20);
		//��ipͷ�ĵ�ַ��ʼ������20���ֽڵ�������check
		memcpy(check, &ip, 20);

		//����IPͷ��У���
		ip.HeadChecksum = checksum(check, 20);

		cout << "�ܳ��ȣ�" << ntohs(ip.TotalLen) << endl;
		cout << "��ʶ����" << ntohs(ip.Identifier) << endl;
		unsigned short Flags = ntohs(ip.Flags);
		unsigned short FragOffset = Flags;
		cout << "��־λ��" << ((Flags >> 15) & 0x01)
			<< ", DF=" << ((Flags >> 14) & 0x01)
			<< ", MF=" << ((Flags >> 13) & 0x01) << endl;
		cout << "Ƭƫ�ƣ�" << (FragOffset & 0x1fff) << "(8B)" << endl;
		cout << "ͷ��У��ͣ�" <<  ip.HeadChecksum << endl;

		//��20�ֽڵ�ipͷ��д������ļ���
		outfile.write((char*)&ip, 20);

		//�������ļ��У���ȡ��ǰ��Ƭ������
		char * data = new char[frag_data_length];
		infile.read(data, frag_data_length);

		
		cout << "�����ֶΣ�";
		for (int i = 0; i < frag_data_length; i++)
		{
			cout << data[i];
		}
		cout << endl;
		//����д������ļ�
		outfile.write(data, frag_data_length);
		delete data;		
	}


	cout << endl << "IP����Ƭ��װ���" << endl;
	outfile.close();
	infile.close();


}