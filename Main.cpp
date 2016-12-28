#include "common.h"


int qe[] = { 0x5601, 0x3401, 0x1801, 0x0ac1, 0x0521, 0x0221, 0x5601,
0x5401, 0x4801, 0x3801, 0x3001, 0x2401, 0x1c01, 0x1601,
0x5601, 0x5401, 0x5101, 0x4801, 0x3801, 0x3401, 0x3001,
0x2801, 0x2401, 0x2201, 0x1c01, 0x1801, 0x1601, 0x1401,
0x1201, 0x1101, 0x0ac1, 0x09c1, 0x08a1, 0x0521, 0x0441,
0x02a1, 0x0221, 0x0141, 0x0111, 0x0085, 0x0049, 0x0025,
0x0015, 0x0009, 0x0005, 0x0001, 0x5601 };

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