#pragma once
#include "common.h"

class CodeBlock {
public:

	CodeBlock();
	void initDataSet();

	int msbSkipped;
	int truncationPointsAmount;
	int lblock;
	int cbLen;
	int m, n; // coords in subband
	int w, h, ulx, uly;
	int offset, scanw;
	int *out_data;
	int *data; 
};