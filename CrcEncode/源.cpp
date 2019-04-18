#include <fstream>
#include <iostream>
#include "源.h"

using std::ios;
using std::cout;
using std::endl;
using std::fstream;
using std::hex;
using std::dec;
/*
ch-4, Ethernet 帧校验程序
*/
int main(int argc, char * argv[])
{
	if (argc!=2)
	{
		cout << endl << "请按以下格式输入命令行：CrcEncode output_file" << endl;
		return 1;
	}

	fstream outfile;  //创建输出文件流
	outfile.open(argv[1], ios::out | ios::binary); //打开输出文件
	for (int i = 0; i < 7; i++)                    //写入7B前导码
	{
		outfile.put(char(0xaa));        
	}

	outfile.put(char(0xab));                       //写入帧前定界符

	int pos_destination_addr = int(outfile.tellp());         //目的地址字段的位置
	ProcessDestinationAddr(outfile);               //处理目的地址
	ProcessSourceAddr(outfile);                   //处理源地址
	ProcessLengthAndData(outfile);                //处理长度字段和数据字段

	int pos_crc = int(outfile.tellp());             //获得帧校验字段的位置
	outfile.put(char(0x00));                      //数据后补8个0
	int total = int(outfile.tellp()) - pos_destination_addr;   //CRC校验范围的字节数
	outfile.seekg(pos_destination_addr, ios::beg);   //文件（读）指针指向目的地址
	unsigned char crc = 0;                      //初始化CRC余数为0
	bool xorflag = false;						//是否进行异或运算的标志
	while (total--)
	{
		char temp;
		outfile.get(temp);   //每次读取一个字节

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

	outfile.seekp(pos_crc, ios::beg);     //文件（写）指针移动到帧校验字段
	outfile.put(crc);
	cout << "帧校验字段："<<hex << (int)crc << dec << "(" << (int)crc << ")" << endl;		
	cout << endl << "帧封装与CRC校验完成" << endl;
	outfile.close();

	return 0;
}

/**
C++ 函数传参：

(1)将变量名作为实参和形参。这时传给形参的是变量的值，传递是单向的。
如果在执行函数期间形参的值发生变化，并不传回给实参。因为在调用函数时，
形参和实参不是同一个存储单元。// 同 c

(2) 传递变量的指针。形参是指针变量，实参是一个变量的地址，调用函数时，
形参(指针变量)指向实参变量单元。这种通过形参指针可以改变实参的值。// 同 c

(3) C++提供了 传递变量的引用(&变量)。形参是引用变量，和实参是一个变量，
调用函数时，形参(引用变量)指向实参变量单元。这种通过形参引用可以改变实参的值。
*/
void MoveNewBitToCrc(unsigned char &crc, char temp, unsigned char i)
{
	crc <<= 1;
	if (unsigned char(temp) & i)    //如果当前新读取的位是1
		crc ^= 0x01;
}

/*
处理目的地址
*/
void ProcessDestinationAddr(std::fstream &outfile)
{

	char destination_addr[6] = { char(0x00),char(0x00),
		char(0xe4),char(0x86),
		char(0x3a),char(0xdc) }; //目的地址

	outfile.write(destination_addr, sizeof(destination_addr)); //写入6B目的地址
}

/*
处理源地址
*/
void ProcessSourceAddr(std::fstream &outfile)
{
	char source_addr[6] = { char(0x00), char(0x00),
		char(0x80), char(0x1a),
		char(0xe6), char(0x65) }; //源地址

	outfile.write(source_addr, sizeof(source_addr)); //写入6B源地址
}

/*
处理长度和数据字段
*/
void ProcessLengthAndData(std::fstream &outfile)
{
	char data[] = "Hello world!";				//数据
	int data_length = strlen(data);				//获得数据的长度
	outfile.put(char(data_length / 256));	     //写入长度字段的首字节
	outfile.put(char(data_length % 256));        //写入长度字段的尾字节
	cout << endl << "长度字段：" << hex << data_length
		<< dec << "(" << data_length << ")" << endl;

	outfile.write(data, data_length);    //写入数据字段
	cout << "数据字段：" << data << endl;
}
