#pragma once
#include "common.h"

class PacketDecoder {
public:
	CodeBlock **** readData(MetadataReader *);
private:
	StreamReader *in;
};