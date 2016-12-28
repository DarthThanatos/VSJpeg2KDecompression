#pragma once
#include "common.h"

class MQDecoder {
public:
	MQDecoder(StreamReader *, int nrOfContexts, int initStates[]);
	int mps_exchange(int cx);
	int lps_exchange(int cx);
	void renormd();
	int decodeSymbol(int context);
	void byteIn();
	void resetCtxts();
	void nextSegment();
	void init();

private:
	int qe[47] = { 0x5601, 0x3401, 0x1801, 0x0ac1, 0x0521, 0x0221, 0x5601,
		0x5401, 0x4801, 0x3801, 0x3001, 0x2401, 0x1c01, 0x1601,
		0x5601, 0x5401, 0x5101, 0x4801, 0x3801, 0x3401, 0x3001,
		0x2801, 0x2401, 0x2201, 0x1c01, 0x1801, 0x1601, 0x1401,
		0x1201, 0x1101, 0x0ac1, 0x09c1, 0x08a1, 0x0521, 0x0441,
		0x02a1, 0x0221, 0x0141, 0x0111, 0x0085, 0x0049, 0x0025,
		0x0015, 0x0009, 0x0005, 0x0001, 0x5601 };

	int nMPS[47] = { 1 , 2, 3, 4, 5,38, 7, 8, 9,10,11,12,13,29,15,16,17,
		18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,
		35,36,37,38,39,40,41,42,43,44,45,45,46 };

	int nLPS[47] = { 1 , 6, 9,12,29,33, 6,14,14,14,17,18,20,21,14,14,15,
		16,17,18,19,19,20,21,22,23,24,25,26,27,28,29,30,31,
		32,33,34,35,36,37,38,39,40,41,42,43,46 };


	int switchLM[47] = { 1,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	
	/** The current most probable signal for each context */
	int *mPS;

	/** The current index of each context */
	int *I;

	/** The current bit code */
	int c;

	/** The bit code counter */
	int cT;

	/** The current interval */
	int a;
	int b;
	int nrOfCtxts;
	boolean markerFound;
	int *initStates;
	StreamReader *in;
};