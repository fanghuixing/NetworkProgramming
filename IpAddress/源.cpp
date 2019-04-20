#include <ctype.h>//用于测试字符是否属于特定的字符类别，如字母字符、控制字符等
#include <math.h>
#include <string>
#include <iostream>
#include "源.h"
using std::cout;
using std::endl;

int main(int argc, char * argv[])
{
	if (argc != 2)  //检查命令行参数
	{
		cout << endl << "请按以下格式输入命令行：IpAddress ip_address" << endl;
		return 1;
	}

	int inputIpLen = strlen(argv[1]);

	char * ipaddr = new char[inputIpLen];
	//字符串复制
	strcpy_s(ipaddr, inputIpLen +1, argv[1]); //从命令行复制IP地址

	cout << endl << "开始分析IP地址";

	
	//检查长度是否超过15
	if (checkIPLength(ipaddr)) return 1;


	//检查IP地址每位合法性
	int dotnum = 0; //分隔符数量

	//检查是否有非法字符，并统计分隔符的数量
	if (checkCharInIpAddress(ipaddr, dotnum)) return 1;


	if (dotnum != 3)
	{
		cout << endl << "IP地址中分隔符只能是3个！" << endl;
		return 1;
	}

	//检查是否有连续的分隔符
	if( checkContinuesSeperator(ipaddr)) return 1;

	int len = strlen(ipaddr);
	//检查最后的字符是否是分隔符
	if( checkLastSeperator(ipaddr, len)) return 1;

	char temp[4][15];		//IP地址缓冲区
	initialTemp(temp);      //初始化

	//划分4段，分别存放在temp的4行中
	partionIntoTemp(ipaddr, temp);

	int ip[4] = { 0,0,0,0 };//点分十进制IP地址

	//检查每段的长度
	if( checkLenOfEachPart(len, temp, ip)) return 1;

	//检查每段的数值是否超过255
	if( checkValueBoundaryOfEachPart(ip)) return 1;

	if (ip[0]<1)
	{
		cout << endl << "IP地址首位不能为0！" << endl;
		return 1;
	}
	//IP地址归类
	classifyIpAddress(ip, argv[1]);
	return 0;
}

void classifyIpAddress(int  ip[4], char * inputIp)
{
	if (ip[0] >= 1 && ip[0] <= 127)
	{
		cout << endl << inputIp << "：A类地址" << endl;
	}
	else if (ip[0] >= 128 && ip[0] <= 191)
	{
		cout << endl << inputIp << "：B类地址" << endl;
	}
	else if (ip[0] >= 192 && ip[0] <= 223)
	{
		cout << endl << inputIp << "：C类地址" << endl;
	}
	else
	{
		cout << endl << inputIp << "：其他类型地址" << endl;
	}
}

int checkValueBoundaryOfEachPart(int  ip[4])
{

	for (int i = 0; i < 4; i++)
	{
		//检查IP地址每位数字
		if (ip[i]>255)
		{
			cout << endl << "IP地址数字不能超过255!" << endl;
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
		//检查IP地址每段长度
		if (len>3)
		{
			cout << endl << "IP地址每位长度不能超过3！" << endl;
			return 1;
		}

		ip[i] = strtol(temp[i], NULL, 10);
	}
	return 0;
}

void partionIntoTemp(char * ipaddr, char  temp[4][15])
{
	int i = 0, j = 0;
	//ipaddr分段存放在temp中
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
	//用结束标志填充
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
		cout << endl << "IP地址最后位不能是分隔符!" << endl;
		return 1;
	}
	return 0;
}

int checkContinuesSeperator(char * ipaddr)
{
	//检查分隔符是否连续
	for (unsigned int i = 0; i < strlen(ipaddr) - 1; i++)
	{
		if (ipaddr[i] == '.' && ipaddr[i + 1] == '.')
		{
			cout << endl << "IP地址中出现连续分隔符！" << endl;
			return 1;
		}

	}
	return 0;
}

int checkCharInIpAddress(char * ipaddr, int &dotnum)
{
	
	for (unsigned int i = 0; i < strlen(ipaddr); i++)
	{
		//isdigit检查所传的字符是否是十进制数字字符；如果不是数字则返回0
		if (isdigit(ipaddr[i]) == 0 && ipaddr[i] != '.')
		{
			cout << endl << "IP地址中含有非法字符！" << endl;
			return 1;
		}

		if (ipaddr[i] == '.')
		{
			dotnum++; //累加分隔符数量
		}
	}
	
	return 0;
}


int checkIPLength(char * ipaddr)
{
	
	//检查IP地址长度
	if (strlen(ipaddr)>15)
	{
		cout << endl << "IP地址总长度不能超过15！" << endl;
		return 1;
	}
	
	return 0;
}
