#include <ctype.h>//���ڲ����ַ��Ƿ������ض����ַ��������ĸ�ַ��������ַ���
#include <math.h>
#include <string>
#include <iostream>
#include "Դ.h"
using std::cout;
using std::endl;

int main(int argc, char * argv[])
{
	if (argc != 2)  //��������в���
	{
		cout << endl << "�밴���¸�ʽ���������У�IpAddress ip_address" << endl;
		return 1;
	}

	int inputIpLen = strlen(argv[1]);

	char * ipaddr = new char[inputIpLen];
	//�ַ�������
	strcpy_s(ipaddr, inputIpLen +1, argv[1]); //�������и���IP��ַ

	cout << endl << "��ʼ����IP��ַ";

	
	//��鳤���Ƿ񳬹�15
	if (checkIPLength(ipaddr)) return 1;


	//���IP��ַÿλ�Ϸ���
	int dotnum = 0; //�ָ�������

	//����Ƿ��зǷ��ַ�����ͳ�Ʒָ���������
	if (checkCharInIpAddress(ipaddr, dotnum)) return 1;


	if (dotnum != 3)
	{
		cout << endl << "IP��ַ�зָ���ֻ����3����" << endl;
		return 1;
	}

	//����Ƿ��������ķָ���
	if( checkContinuesSeperator(ipaddr)) return 1;

	int len = strlen(ipaddr);
	//��������ַ��Ƿ��Ƿָ���
	if( checkLastSeperator(ipaddr, len)) return 1;

	char temp[4][15];		//IP��ַ������
	initialTemp(temp);      //��ʼ��

	//����4�Σ��ֱ�����temp��4����
	partionIntoTemp(ipaddr, temp);

	int ip[4] = { 0,0,0,0 };//���ʮ����IP��ַ

	//���ÿ�εĳ���
	if( checkLenOfEachPart(len, temp, ip)) return 1;

	//���ÿ�ε���ֵ�Ƿ񳬹�255
	if( checkValueBoundaryOfEachPart(ip)) return 1;

	if (ip[0]<1)
	{
		cout << endl << "IP��ַ��λ����Ϊ0��" << endl;
		return 1;
	}
	//IP��ַ����
	classifyIpAddress(ip, argv[1]);
	return 0;
}

void classifyIpAddress(int  ip[4], char * inputIp)
{
	if (ip[0] >= 1 && ip[0] <= 127)
	{
		cout << endl << inputIp << "��A���ַ" << endl;
	}
	else if (ip[0] >= 128 && ip[0] <= 191)
	{
		cout << endl << inputIp << "��B���ַ" << endl;
	}
	else if (ip[0] >= 192 && ip[0] <= 223)
	{
		cout << endl << inputIp << "��C���ַ" << endl;
	}
	else
	{
		cout << endl << inputIp << "���������͵�ַ" << endl;
	}
}

int checkValueBoundaryOfEachPart(int  ip[4])
{

	for (int i = 0; i < 4; i++)
	{
		//���IP��ַÿλ����
		if (ip[i]>255)
		{
			cout << endl << "IP��ַ���ֲ��ܳ���255!" << endl;
			return 1;
		}
	}
	return 0;
}

int checkLenOfEachPart(int &len, char  temp[4][15], int  ip[4])
{
	for (int i = 0; i < 4; i++)
	{
		len = strlen(temp[i]);
		//���IP��ַÿ�γ���
		if (len>3)
		{
			cout << endl << "IP��ַÿλ���Ȳ��ܳ���3��" << endl;
			return 1;
		}

		ip[i] = strtol(temp[i], NULL, 10);
	}
	return 0;
}

void partionIntoTemp(char * ipaddr, char  temp[4][15])
{
	int i = 0, j = 0;
	//ipaddr�ֶδ����temp��
	for (unsigned int k = 0; k < strlen(ipaddr); k++)
	{
		if (ipaddr[k] != '.')
		{
			temp[i][j] = ipaddr[k];
			j++;
		}
		else
		{
			i++;
			j = 0;
		}
	}
}

void initialTemp(char  temp[4][15])
{
	//�ý�����־���
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 15; j++)
		{
			temp[i][j] = '\0';
		}
	}
}

int checkLastSeperator(char * ipaddr, int len)
{
	if (ipaddr[len - 1] == '.')
	{
		cout << endl << "IP��ַ���λ�����Ƿָ���!" << endl;
		return 1;
	}
	return 0;
}

int checkContinuesSeperator(char * ipaddr)
{
	//���ָ����Ƿ�����
	for (unsigned int i = 0; i < strlen(ipaddr) - 1; i++)
	{
		if (ipaddr[i] == '.' && ipaddr[i + 1] == '.')
		{
			cout << endl << "IP��ַ�г��������ָ�����" << endl;
			return 1;
		}

	}
	return 0;
}

int checkCharInIpAddress(char * ipaddr, int &dotnum)
{
	
	for (unsigned int i = 0; i < strlen(ipaddr); i++)
	{
		//isdigit����������ַ��Ƿ���ʮ���������ַ���������������򷵻�0
		if (isdigit(ipaddr[i]) == 0 && ipaddr[i] != '.')
		{
			cout << endl << "IP��ַ�к��зǷ��ַ���" << endl;
			return 1;
		}

		if (ipaddr[i] == '.')
		{
			dotnum++; //�ۼӷָ�������
		}
	}
	
	return 0;
}


int checkIPLength(char * ipaddr)
{
	
	//���IP��ַ����
	if (strlen(ipaddr)>15)
	{
		cout << endl << "IP��ַ�ܳ��Ȳ��ܳ���15��" << endl;
		return 1;
	}
	
	return 0;
}
