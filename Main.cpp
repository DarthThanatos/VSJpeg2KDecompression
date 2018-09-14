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

void dequantize(MetadataReader *mr, CodeBlock**** cblks) {
	for (int c = 0; c < ALL_C; c++) {
		int i = 0;
		for (int r = 0; r < ALL_R; r++) {
			int mins = (r == 0) ? 0 : 1;
			int maxs = (r == 0) ? 1 : 4;
			for (int s = mins; s < maxs; s++) {
				CodeBlock *cblk = cblks[c][r][s];
				int mb = mr->guard_bits + mr->eps[i] - 1;
				int shiftBits = 31 - mb;
				// We decoded all bits => Nb(u,v) == mb and coefficients are already reconstructed (no quantization in the reversible transform is performed).
				// However, for the convinience (and speed) in the entropy decoding phase, consecutive bitplanes are appended in a specific way:
				// e.g. if a coefficient decompressed so far during past passes is now xxxx00000...0 (four bits decoded), 5th bit y was going to be 
				// appended after fourth bit to the left: xxxxy00..0. The sequence of zeros following decompressed bits has the length exactly 31 - mb,
				// thus the shift to the right by 31 - mb positions (as xxx...x has the mb length and the number can have max 32 bits of precision).   
				for (int j = 0; j < cblk->w * cblk->h; j++) {
					int coeff = cblk->coefficients[j]; 
					cblk->coefficients[j] = (coeff >= 0) ? (coeff >> shiftBits) : -((coeff & 0x7FFFFFFF) >> shiftBits);
				}
				i++;
			}
		}
	}
}

float min_(float first, float second) {
	return first < second ? first : second;
}

int*** irct(MetadataReader *mr, int **I) {
	int ***RGB = new int**[ALL_C];

	for (int c = 0; c < ALL_C; c++) {
		RGB[c] = new int*[mr->Ysiz];
		for (int i = 0; i < mr->Ysiz; i++) {
			RGB[c][i] = new int[mr->Xsiz];
			for (int j = 0; j < mr->Xsiz; j++) {
				RGB[c][i][j] = 0;
			}
		}
	}

	for (int i = 0; i < mr->Ysiz; i++) {
		for (int j = 0; j < mr->Xsiz; j++) {
			RGB[1][i][j] = min_(I[0][i * mr->Xsiz + j] - floor((I[2][i * mr->Xsiz + j] + I[1][i * mr->Xsiz + j])/4.0), 255);
			RGB[0][i][j] = min_(I[2][i * mr->Xsiz + j] + RGB[1][i][j], 255);
			RGB[2][i][j] = min_(I[1][i * mr->Xsiz + j] + RGB[1][i][j], 255);
			for (int k = 0; k < 3; k++) {
				RGB[k][i][j] = max(RGB[k][i][j] + 128, 0);
			}
		}
	}

	return RGB;

}

void writeToFile(MetadataReader *mr, int ***RGB) {
	FILE* res_file = fopen("result.txt", "wb");
	fprintf(res_file, "%d %d\n", mr->Xsiz, mr->Ysiz);
	for (int k = 0; k < 3; k++) {
		for (int i = 0; i < mr->Ysiz; i++) {
			for (int j = 0; j < mr->Xsiz; j++) {
				fprintf(res_file, "%d ", (int)RGB[k][i][j]);
			}
			fprintf(res_file, "\n");
		}
		fprintf(res_file, "\n");
	}
	fclose(res_file);
	cout << "done" << endl;
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
	dequantize(metadataReader, cblks);
	InverseWaveletTransform *iwt = new InverseWaveletTransform(cblks, metadataReader);
	int **imgComponents = iwt->inverseSubbandCodeblocks();
	int ***rgb = irct(metadataReader, imgComponents);
	writeToFile(metadataReader, rgb);
}

void test() {
	int c = -1926529024;
	c = 542256977;
	unsigned int c_s = c;
	bitset<32> x(c), x_s(c_s >> 16);
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
	//printf("Press any key to continue\n");
	//getchar();
	return 0;
}