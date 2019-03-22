#include "string.h"
#include "fstream"
#include "iostream"

using namespace std;


//fstream ����û�и��ƹ��캯������˲���ִ�и��Ʋ�������������ʵ�θ��Ƹ��βΣ�
//������Ǳ���ʹ�����õķ�ʽ(&)���ݲ���
/**
��ӡĿ�ĵ�ַ����Դ��ַ
*/
void printAddress(const char * info, fstream& infile) {
	cout <<endl<< info;
	//Ŀ��/Դ��ַ����6���ֽ�
	for (int i = 0; i < 6; i++)
	{
		cout << hex << infile.get() << dec;
		if (i != 5) cout << "-"; //�м���'-'�ָ�
	}

}



/**
֡�������ֶ��100���ֽڣ���֡����
*/
int main(int argc, char* argv[])
{
	if (argc != 3) //��������в���
	{
		cout<<endl<<"�밴���¸�ʽ����������:FrameParse input_file"
				  <<" out_file"<<endl;
		return 1;
	}

	fstream outfile; //��������ļ���
	outfile.open(argv[2], ios::out | ios::binary); //������ļ�

	fstream infile; //���������ļ���
	infile.open(argv[1], ios::in | ios::binary); //�������ļ�

	if (!infile.is_open())
	{
		cout << "�޷��������ļ�" << endl;
		return 1;
	}

	bool bframe = true; //�Ƿ���֡��Ҫ����
	int nframes = 0;//�ļ��е�ǰ��д���ַ�/�ֽڵ����λ��
	int nframenum = 0;//֡���
	int nframelen = 0;//���ݳ���

	while (bframe)
	{
		int i = 0;
		nframenum++;
		cout << endl << "֡" << nframenum << "��ʼ����" << endl;

		//tellg: Returns the position of the current character in the input stream.
		nframes = infile.tellg();//�õ����µĿ�ʼλ��

		//����7Bǰ����
		for (int i = 0; i < 7; i++)
		{
			//get: ���ļ��ж�ȡһ���ֽڵ��ڴ�
			if (infile.get()!=0xaa)
			{
				cout << "û���ҵ��Ϸ���֡" << endl;
				infile.close();
				return 1;
			}
		}
		//ǰ���������
		//�ж�1B��֡ǰ�����
		if (infile.get() != 0xab)
		{
			cout << "û���ҵ��Ϸ���֡" << endl;
			infile.close();
			return 1;
		}
		//seekg�Ƕ������ļ���λ����������������
		//��һ��������ƫ�������ڶ��������ǻ���ַ
		infile.seekg(nframes, ios::beg);
		cout << endl << "ǰ���룺";
		for (int i = 0; i < 7; i++)
		{
			cout << hex << infile.get() << dec << " ";
		}
		
		cout << endl << "֡ǰ�������";
		cout << hex << infile.get()<<endl;

		
		printAddress("Ŀ�ĵ�ַ��", infile);
		printAddress("Դ��ַ��", infile);

		cout << endl << "�����ֶ�:";
		//��ȡ�����ֶεĸ�8λ
		cout << hex << infile.get() << " ";
		//��ȡ�����ֶεĵ�8λ
		nframelen = infile.get();
		cout << hex << nframelen;

		char * data = new char[nframelen];
		//�������ļ������ݣ�nframelen���ֽڣ���data��
		infile.read(data, nframelen);
		outfile.write(data, nframelen);
		cout << endl << "�����ֶΣ�";
		for (i = 0; i < nframelen; i++)
		{
			cout << data[i];
		}
		delete data; 
		//�������͵Ķ���û���������������Ի��ջ���������ɵ�����ռ��� 
		//delete �� delete[] ���ǿ��Ե�

		if (nframelen < 100)
			bframe = false;

		if (nframelen < 46)
		{
			for (i = 0; i < 46 - nframelen; i++)
				infile.get();
		}

		cout << endl << "֡У���ֶΣ�";
		for (i = 0; i < 4; i++)
		{
			cout << hex << infile.get() << dec << " ";
		}

		cout << endl;

	}

	cout << endl << "֡ȫ���������" << endl;
	//�ر��ļ�
	outfile.close();
	infile.close();

	return 0;
}



