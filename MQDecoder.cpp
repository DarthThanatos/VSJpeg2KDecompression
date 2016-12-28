#include "common.h"



MQDecoder :: MQDecoder(StreamReader *iStream, int nrOfContexts,	int initStates[]) {
	in = iStream;
	I = new int[nrOfContexts];
	mPS = new int[nrOfContexts];
	// Save the initial states
	this->initStates = initStates;
	init();
	this->nrOfCtxts = nrOfContexts;
	resetCtxts();
}

int MQDecoder :: mps_exchange(int cx) {
	int d;
	if (a < qe[I[cx]]) {
		d = 1 - mPS[cx];
		if (switchLM[I[cx]] == 1) {
			mPS[cx] = 1 - mPS[cx];
		}
		I[cx] = nLPS[I[cx]];
	}
	else {
		d = mPS[cx];
		I[cx] = nMPS[I[cx]];
	}

	return d;
}

int MQDecoder :: lps_exchange(int cx) {
	int d;
	if (a < qe[I[cx]]) {
		a = qe[I[cx]];
		d = mPS[cx];
		I[cx] = nMPS[I[cx]];
	}
	else {
		a = qe[I[cx]];
		d = 1 - mPS[cx];
		if (switchLM[I[cx]] == 1) {
			mPS[cx] = 1 - mPS[cx];
		}
		I[cx] = nLPS[I[cx]];
	}
	return d;
}

void MQDecoder::renormd() {

	do {
		if (cT == 0)
			byteIn();
		a <<= 1;
		c <<= 1;
		cT--;
	} while ((a & 0x8000) == 0);

}

int MQDecoder::decodeSymbol(int context) {
	int q;
	int la;
	int index;
	int decision;

	index = I[context];
	q = qe[index];

	a -= qe[I[context]];
	if ((c >>  16) < a) {

		if ((a & 0x8000) == 0) {
			decision = mps_exchange(context);
			renormd();
		}
		else {
			decision = mPS[context];
		}
	}
	else {
		c -= (a << 16);
		decision = lps_exchange(context);
		renormd();

	}
	return decision;
}


void MQDecoder::byteIn() {
	if (b == 0xFF) {
		b = in->readByte() & 0xFF; // Convert EOFs (-1) to 0xFF

		if (b>0x8F) {
			markerFound = true;
			// software-convention decoder: c unchanged
			cT = 8;
		}
		else {
			c += 0xFE00 - (b << 9);
			cT = 7;
		}
	}
	else {
		b = in->readByte() & 0xFF; // Convert EOFs (-1) to 0xFF
		c += 0xFF00 - (b << 8);
		cT = 8;
	}
}

void MQDecoder::resetCtxts() {
	for (int i = 0; i < nrOfCtxts; i++) {
		initStates[i] = 0;
		mPS[i] = 0;
	}
}

void MQDecoder::nextSegment() {
	init();
}




void MQDecoder::init() {
	// --- INITDEC
	markerFound = false;

	// Read first byte
	b = in->readByte() & 0xFF;

	// Software conventions decoder
	c = (b ^ 0xFF);
	c <<= 16;
	byteIn();
	c = c << 7;
	cT = cT - 7;
	a = 0x8000;
}