#pragma once
#include "common.h"

class Subband {
	public:
		Subband();

		int w, h, ulcx, ulcy, lvls, gOrient;
		Subband *LL, *HL, *LH, *HH;
		Subband *parent;
		int subRange[2];
		list<CodeBlock*>* cblocks;
};