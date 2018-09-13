#include "common.h"

StreamReader::StreamReader(char *input_name, char * output_name) {
	input = fopen(input_name, "rb");
	output = fopen(output_name, "wb");
}

unsigned char * StreamReader::readNBytes(unsigned char* buffer, int N) {
	fread(buffer, N, 1, input);
	return buffer;
}

unsigned char StreamReader::readByte() {
	unsigned char byte;
	fread(&byte, 1, 1, input);
	return byte;
}

unsigned short StreamReader::readTwoBytes() {
	unsigned short result = 0;
	unsigned short result_part = 0;
	fread(&result_part, 1, 1, input);
	result |= (result_part) << 8;
	fread(&result_part, 1, 1, input);
	result |= (result_part & 0x00ff);
	return result;
}

unsigned short StreamReader::getTwoByteLength() {
	unsigned short length = readTwoBytes();
	fprintf(output, "\n%hu //length\n", length);
	return length;
}

unsigned int StreamReader::readFourBytes() {
	unsigned int result = 0;
	unsigned int result_part = 0;
	for (int i = 24; i >= 0; i -= 8) {
		fread(&result_part, 1, 1, input);
		result |= result_part << i;
	}
	return result;
}

int StreamReader::readBit() {
	return 0; //to-do
}
int StreamReader::readBits(int n) {
	return 0; //to-do
}

StreamReader::~StreamReader() {
	fclose(input);
	fclose(output);
}