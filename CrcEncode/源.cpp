#include <fstream>
#include <iostream>
#include "Դ.h"

using std::ios;
using std::cout;
using std::endl;
using std::fstream;
using std::hex;
using std::dec;
/*
ch-4, Ethernet ֡У�����
*/
int main(int argc, char * argv[])
{
	if (argc!=2)
	{
		cout << endl << "�밴���¸�ʽ���������У�CrcEncode output_file" << endl;
		return 1;
	}

	fstream outfile;  //��������ļ���
	outfile.open(argv[1], ios::out | ios::binary); //������ļ�
	for (int i = 0; i < 7; i++)                    //д��7Bǰ����
	{
		outfile.put(char(0xaa));        
	}

	outfile.put(char(0xab));                       //д��֡ǰ�����

	int pos_destination_addr = int(outfile.tellp());         //Ŀ�ĵ�ַ�ֶε�λ��
	ProcessDestinationAddr(outfile);               //����Ŀ�ĵ�ַ
	ProcessSourceAddr(outfile);                   //����Դ��ַ
	ProcessLengthAndData(outfile);                //�������ֶκ������ֶ�

	int pos_crc = int(outfile.tellp());             //���֡У���ֶε�λ��
	outfile.put(char(0x00));                      //���ݺ�8��0
	int total = int(outfile.tellp()) - pos_destination_addr;   //CRCУ�鷶Χ���ֽ���
	outfile.seekg(pos_destination_addr, ios::beg);   //�ļ�������ָ��ָ��Ŀ�ĵ�ַ
	unsigned char crc = 0;                      //��ʼ��CRC����Ϊ0
	bool xorflag = false;						//�Ƿ�����������ı�־
	while (total--)
	{
		char temp;
		outfile.get(temp);   //ÿ�ζ�ȡһ���ֽ�

		for (unsigned char i = (unsigned char) 0x80; i > 0; i>>=1)
		{
			xorflag = false;
			if (crc & 0x80) {
				xorflag = true;
			}
			MoveNewBitToCrc(crc, temp, i);
			if (xorflag) {
				crc ^= 0x07;
			}					
		}
	}

	outfile.seekp(pos_crc, ios::beg);     //�ļ���д��ָ���ƶ���֡У���ֶ�
	outfile.put(crc);
	cout << "֡У���ֶΣ�"<<hex << (int)crc << dec << "(" << (int)crc << ")" << endl;		
	cout << endl << "֡��װ��CRCУ�����" << endl;
	outfile.close();

	return 0;
}

/**
C++ �������Σ�

(1)����������Ϊʵ�κ��βΡ���ʱ�����βε��Ǳ�����ֵ�������ǵ���ġ�
�����ִ�к����ڼ��βε�ֵ�����仯���������ظ�ʵ�Ρ���Ϊ�ڵ��ú���ʱ��
�βκ�ʵ�β���ͬһ���洢��Ԫ��// ͬ c

(2) ���ݱ�����ָ�롣�β���ָ�������ʵ����һ�������ĵ�ַ�����ú���ʱ��
�β�(ָ�����)ָ��ʵ�α�����Ԫ������ͨ���β�ָ����Ըı�ʵ�ε�ֵ��// ͬ c

(3) C++�ṩ�� ���ݱ���������(&����)���β������ñ�������ʵ����һ��������
���ú���ʱ���β�(���ñ���)ָ��ʵ�α�����Ԫ������ͨ���β����ÿ��Ըı�ʵ�ε�ֵ��
*/
void MoveNewBitToCrc(unsigned char &crc, char temp, unsigned char i)
{
	crc <<= 1;
	if (unsigned char(temp) & i)    //�����ǰ�¶�ȡ��λ��1
		crc ^= 0x01;
}

/*
����Ŀ�ĵ�ַ
*/
void ProcessDestinationAddr(std::fstream &outfile)
{

	char destination_addr[6] = { char(0x00),char(0x00),
		char(0xe4),char(0x86),
		char(0x3a),char(0xdc) }; //Ŀ�ĵ�ַ

	outfile.write(destination_addr, sizeof(destination_addr)); //д��6BĿ�ĵ�ַ
}

/*
����Դ��ַ
*/
void ProcessSourceAddr(std::fstream &outfile)
{
	char source_addr[6] = { char(0x00), char(0x00),
		char(0x80), char(0x1a),
		char(0xe6), char(0x65) }; //Դ��ַ

	outfile.write(source_addr, sizeof(source_addr)); //д��6BԴ��ַ
}

/*
�����Ⱥ������ֶ�
*/
void ProcessLengthAndData(std::fstream &outfile)
{
	char data[] = "Hello world!";				//����
	int data_length = strlen(data);				//������ݵĳ���
	outfile.put(char(data_length / 256));	     //д�볤���ֶε����ֽ�
	outfile.put(char(data_length % 256));        //д�볤���ֶε�β�ֽ�
	cout << endl << "�����ֶΣ�" << hex << data_length
		<< dec << "(" << data_length << ")" << endl;

	outfile.write(data, data_length);    //д�������ֶ�
	cout << "�����ֶΣ�" << data << endl;
}
