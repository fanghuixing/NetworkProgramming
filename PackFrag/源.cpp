#include <string>
#include <WinSock2.h>
#include <fstream>
#include <iostream>

using std::endl;
using std::cout;
using std::fstream;
using std::ios;

#pragma comment (lib, "ws2_32")

//IP头部结构
typedef struct IP_HEAD
{
	union
	{
		unsigned char Version; //版本(字节前4位)
		unsigned char HeadLen; //头部长度(字节后4位)
	};

	unsigned char ServiceType;		//服务类型

	unsigned short TotalLen;		//总长度

	unsigned short Identifier;		//标识符

	union
	{
		unsigned short Flags;		//标志位(字前3位)
		unsigned short FragOffset;	//片偏移(字后13位)
	};

	unsigned char TimeToLive;		//生存周期

	unsigned char Protocol;			//协议

	unsigned short HeadChecksum;	//头部校验和

	unsigned int SourceAddr;		//源IP地址

	unsigned int DestinAddr;		//目的IP地址

}ip_head;


/*
buffer : IP头部中，参加运算的16位数值
size： 参加运算的IP头部总字节数量
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
	//检查命令行参数
	if (argc != 3)
	{
		cout << endl << "请按以下格式输入命令行：PackFrag input_file output_file"
			<< endl;
		return;
	}

	//输出文件
	fstream outfile;
	outfile.open(argv[2], ios::out);

	fstream infile;
	infile.open(argv[1], ios::in);

	if (!infile.is_open())
	{
		cout << endl << "无法打开输入文件" << endl;
		return;
	}

	infile.seekg(0, ios::end); //文件（读）指针指向结尾
	//获得输入文件长度，待分片的数据长度一开始等于文件长度
	int data_length = infile.tellg(); 
	infile.seekg(0, ios::beg);	//文件指针指向开始

	bool is_more_frag = true; //用来判断是否继续分片
	int frag_data_length = 0;//初始化分片数据长度
	int frag_num = 0;//初始化分片序号
	unsigned short check[10];//设置校验缓冲区大小
	while (is_more_frag)
	{
		ip_head ip = { 0 }; //初始化ip头
		
		frag_num++; //分片序号加1
		cout << endl << "IP包" << frag_num << "开始封装" << endl;
		//如果待分片的数据少于80字节，则应该是到最后一个分片了；否则还有更多分片
		if (data_length < 80) 
		{
			is_more_frag = false;//到了最后一个分片啦
			frag_data_length = data_length; //设置当前分片长度
			//设置标志为000，低字节存放偏移量
			ip.Flags = unsigned short((frag_num-1)*10); //(80/8=10)
		}
		else {//不是最后一个分片
			data_length = data_length - 80; //更新数据长度，每个分片分到80个字节
			frag_data_length = 80; //当然分片长度
			//设置标志为001，低字节存放偏移量
			ip.Flags = unsigned short( ((frag_num-1)*10) | 0x2000);
		}
		ip.Flags = htons(ip.Flags);

		//处理IP头部其他字段

		//前四位放整数4，后4位放以4字节为单位额ip头长度
		ip.Version = (0x04 << 4 | sizeof(ip_head) / sizeof(unsigned int));
		ip.ServiceType = unsigned char(0x00);
		//分片数据长度+IP头长度
		ip.TotalLen = htons( unsigned short(frag_data_length + 20)); 
		ip.Identifier = htons(unsigned short(0x1000));
		ip.TimeToLive = unsigned char(0x80); //128
		ip.Protocol = unsigned char(0x06); //tcp
		ip.HeadChecksum = unsigned short(0x00); //初始校验和置0
		ip.SourceAddr = unsigned int(0x3801a8c0);
		ip.DestinAddr = unsigned int(0x3502a8c0);

		//将缓冲区check的首20个字节的值设为值0。
		memset(check, 0, 20);
		//从ip头的地址开始，拷贝20个字节到缓冲区check
		memcpy(check, &ip, 20);

		//计算IP头部校验和
		ip.HeadChecksum = checksum(check, 20);

		cout << "总长度：" << ntohs(ip.TotalLen) << endl;
		cout << "标识符：" << ntohs(ip.Identifier) << endl;
		unsigned short Flags = ntohs(ip.Flags);
		unsigned short FragOffset = Flags;
		cout << "标志位：" << ((Flags >> 15) & 0x01)
			<< ", DF=" << ((Flags >> 14) & 0x01)
			<< ", MF=" << ((Flags >> 13) & 0x01) << endl;
		cout << "片偏移：" << (FragOffset & 0x1fff) << "(8B)" << endl;
		cout << "头部校验和：" <<  ip.HeadChecksum << endl;

		//将20字节的ip头部写入输出文件中
		outfile.write((char*)&ip, 20);

		//从输入文件中，读取当前分片的数据
		char * data = new char[frag_data_length];
		infile.read(data, frag_data_length);

		
		cout << "数据字段：";
		for (int i = 0; i < frag_data_length; i++)
		{
			cout << data[i];
		}
		cout << endl;
		//数据写入输出文件
		outfile.write(data, frag_data_length);
		delete data;		
	}


	cout << endl << "IP包分片封装完成" << endl;
	outfile.close();
	infile.close();


}