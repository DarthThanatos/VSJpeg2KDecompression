#pragma once
#include "common.h"

class TagTreeDecoder {
public:
	TagTreeDecoder(PacketHeaderReader *, int, int);
	int update();
private:
	PacketHeaderReader *in;
	int lvls, w,h;
	int ts, tv;
	int **treeS, **treeV;
};