#include "fstream"
#include "iostream"
using namespace std;

void main() {
	ofstream input;
	input.open("input", ios::out | ios::binary);

	char data[] = "Nankai University was founded in 1919"
		" by the famous patriotic educators in Chinese modern history,"
		" Mr. Zhang Boling and Mr. Yan Xiu.";
	char dst_addr[6] = { char(0x00), char(0x00), char(0xe4),
		char(0x86), char(0x3a), char(0xdc) };
	char src_addr[6] = { char(0x00), char(0x00), char(0x80),
			char(0x1a), char(0xe6), char(0x65) };
	int length = strlen(data);
	int from = 0, to = 0;

	while (length > 0) {
		if (length >= 100) {
			from = to;
			to = from + 100;
			length = length - 100;
		}
		else {
			from = to;
			to = from + length;
			length = 0;
		}


		for (int i = 0; i < 7; i++)
			input.put(char(0xaa));

		input.put(char(0xab));
		input.write(dst_addr, sizeof(dst_addr));
		input.write(src_addr, sizeof(src_addr));
		int datalen = to - from;
		input.put(char(datalen / 256));
		input.put(char(datalen % 256));

		for (int i = from; i < to; i++)
		{
			input.put(data[i]);
			cout << data[i];
		}
		cout << endl;


		if (datalen < 46)
		{
			for (int i = 0; i < 46 - datalen; i++)
				input.put(char(0x00));
		}

		for (int i = 0; i < 4; i++) {
			input.put(char(0xff));
		}
	}
	input.close();

}