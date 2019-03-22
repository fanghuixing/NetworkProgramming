#include "string.h"
#include "fstream"
#include "iostream"

using namespace std;


//fstream 对象没有复制构造函数，因此不能执行复制操作，而函数是实参复制给形参，
//因此我们必须使用引用的方式(&)传递参数
/**
打印目的地址或者源地址
*/
void printAddress(const char * info, fstream& infile) {
	cout <<endl<< info;
	//目的/源地址都有6个字节
	for (int i = 0; i < 6; i++)
	{
		cout << hex << infile.get() << dec;
		if (i != 5) cout << "-"; //中间以'-'分隔
	}

}



/**
帧的数据字段最长100个字节，对帧解析
*/
int main(int argc, char* argv[])
{
	if (argc != 3) //检查命令行参数
	{
		cout<<endl<<"请按如下格式输入命令行:FrameParse input_file"
				  <<" out_file"<<endl;
		return 1;
	}

	fstream outfile; //创建输出文件流
	outfile.open(argv[2], ios::out | ios::binary); //打开输出文件

	fstream infile; //创建输入文件流
	infile.open(argv[1], ios::in | ios::binary); //打开输入文件

	if (!infile.is_open())
	{
		cout << "无法打开输入文件" << endl;
		return 1;
	}

	bool bframe = true; //是否还有帧需要解析
	int nframes = 0;//文件中当前读写的字符/字节的相对位置
	int nframenum = 0;//帧序号
	int nframelen = 0;//数据长度

	while (bframe)
	{
		int i = 0;
		nframenum++;
		cout << endl << "帧" << nframenum << "开始解析" << endl;

		//tellg: Returns the position of the current character in the input stream.
		nframes = infile.tellg();//得到最新的开始位置

		//查找7B前导码
		for (int i = 0; i < 7; i++)
		{
			//get: 在文件中读取一个字节到内存
			if (infile.get()!=0xaa)
			{
				cout << "没有找到合法的帧" << endl;
				infile.close();
				return 1;
			}
		}
		//前导码结束后
		//判断1B的帧前定界符
		if (infile.get() != 0xab)
		{
			cout << "没有找到合法的帧" << endl;
			infile.close();
			return 1;
		}
		//seekg是对输入文件定位，它有两个参数：
		//第一个参数是偏移量，第二个参数是基地址
		infile.seekg(nframes, ios::beg);
		cout << endl << "前导码：";
		for (int i = 0; i < 7; i++)
		{
			cout << hex << infile.get() << dec << " ";
		}
		
		cout << endl << "帧前定界符：";
		cout << hex << infile.get()<<endl;

		
		printAddress("目的地址：", infile);
		printAddress("源地址：", infile);

		cout << endl << "长度字段:";
		//读取长度字段的高8位
		cout << hex << infile.get() << " ";
		//读取长度字段的低8位
		nframelen = infile.get();
		cout << hex << nframelen;

		char * data = new char[nframelen];
		//从输入文件读数据（nframelen个字节）到data中
		infile.read(data, nframelen);
		outfile.write(data, nframelen);
		cout << endl << "数据字段：";
		for (i = 0; i < nframelen; i++)
		{
			cout << data[i];
		}
		delete data; 
		//基本类型的对象没有析构函数，所以回收基本类型组成的数组空间用 
		//delete 和 delete[] 都是可以的

		if (nframelen < 100)
			bframe = false;

		if (nframelen < 46)
		{
			for (i = 0; i < 46 - nframelen; i++)
				infile.get();
		}

		cout << endl << "帧校验字段：";
		for (i = 0; i < 4; i++)
		{
			cout << hex << infile.get() << dec << " ";
		}

		cout << endl;

	}

	cout << endl << "帧全部解析完成" << endl;
	//关闭文件
	outfile.close();
	infile.close();

	return 0;
}



