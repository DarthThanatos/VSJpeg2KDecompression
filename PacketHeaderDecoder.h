#pragma once
#include "common.h"

class PacketHeaderDecoder {
public:
	bool readPktHead(int c, int r, Subband *);
private:
	StreamReader *in;
};