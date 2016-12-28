#pragma once
#include "common.h"

class StreamReader {
public:
	StreamReader(char *input, char* output);
	~StreamReader();
	unsigned char readByte();
	unsigned short readTwoBytes();
	unsigned int readFourBytes();
	unsigned short getTwoByteLength();
	unsigned char *readNBytes(unsigned char*, int);
	int readBit();
	int readBits(int n);

private:
	FILE * input;
	FILE * output;
};