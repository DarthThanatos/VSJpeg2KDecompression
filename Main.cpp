#include "common.h"
#include <bitset>

void print_bin_rek(int number) {
	if (number / 2 != 0) print_bin_rek(number / 2);
	printf("%d", number % 2);
}


void print_bin_padded(int number, char *end_str) {
#define PRECISION 16
	int byte[PRECISION], r = PRECISION - 1;
	for (int i = 0; i < PRECISION; i++)byte[i] = 0;
	while (number != 0) {
		byte[r] = number % 2;
		r--;
		number /= 2;
	}
	for (int i = 0; i <= PRECISION - 1; i++) printf("%d", byte[i]);
	printf("%s", end_str);
}

void decode(int argc, char*argv[]) {
	StreamReader *streamReader = new StreamReader(argv[1], argv[2]);
	MetadataReader *metadataReader = new MetadataReader(streamReader);
	metadataReader->parseMetadata();
	metadataReader->initSubbands();
	PacketDecoder *packetDecoder = new PacketDecoder();
	CodeBlock**** cblks = packetDecoder->readData(metadataReader);
	EntropyDecoder *ed = new EntropyDecoder(metadataReader);
	ed->decode(cblks);

	Subband *s = metadataReader->componentsSubbandRoots[0]->getSubbandAt(5, 0);
	cout << (s != NULL ? s->toString() : "NULL") << endl;

	int c = -1926529024;
	c = 542256977;
	unsigned int c_s = c;
	bitset<32> x(c), x_s(c_s>>16);
	cout << x << " " << x_s << endl;
}


void init(int argc, char*argv[]) {
	if (argc != 3) {
		cout << "Usage: change.exe file.jp2 out_file.txt"<<endl;
		getchar();
		exit(-1);
	}
}


int main(int argc, char*argv[]) {
	init(argc, argv);
	decode(argc, argv);
	printf("Press any key to continue\n");
	getchar();
	return 0;
}