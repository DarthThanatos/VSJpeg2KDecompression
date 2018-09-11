#pragma once
#include "common.h"

class CodeBlock {
public:
	string toString();
	void printCoefficients();

	int cbLen;
	int w, h, oX, oY;
	unsigned char *data; //compressed data read from a packet's body
	int *coefficients; //an array wxh of reconstructed wavelet transform coefficients 
	/** The number of most significant bits which are skipped for this
	* code-block (= Mb-1-bitDepth). */
	int msbSkipped;

	/** The number of truncation point for each layer */
	int ntp;
};