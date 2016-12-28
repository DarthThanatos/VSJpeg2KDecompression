#pragma once
#include "common.h"

class TagTreeDecoder {
public:
	TagTreeDecoder(StreamReader *);
	int update(CodeBlock *);
private:
	StreamReader *in;
	int lvls, w,h;
	int ts, tv;
	int **treeS, **treeV;
};