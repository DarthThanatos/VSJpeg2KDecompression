#include "common.h"


EntropyDecoder::EntropyDecoder(StreamReader*) {
	int i, j;
	double val, deltaMSE;
	int *inter_sc_lut;
	int ds, us, rs, ls;
	int dsgn, usgn, rsgn, lsgn;
	int h, v;

	// Initialize the zero coding lookup tables

	// LH

	// - No neighbors significant
	ZC_LUT_LH[0] = 2;
	// - No horizontal or vertical neighbors significant
	for (i = 1; i<16; i++) { // Two or more diagonal coeffs significant
		ZC_LUT_LH[i] = 4;
	}
	for (i = 0; i<4; i++) { // Only one diagonal coeff significant
		ZC_LUT_LH[1 << i] = 3;
	}
	// - No horizontal neighbors significant, diagonal irrelevant
	for (i = 0; i<16; i++) {
		// Only one vertical coeff significant
		ZC_LUT_LH[STATE_V_U_R1 | i] = 5;
		ZC_LUT_LH[STATE_V_D_R1 | i] = 5;
		// The two vertical coeffs significant
		ZC_LUT_LH[STATE_V_U_R1 | STATE_V_D_R1 | i] = 6;
	}
	// - One horiz. neighbor significant, diagonal/vertical non-significant
	ZC_LUT_LH[STATE_H_L_R1] = 7;
	ZC_LUT_LH[STATE_H_R_R1] = 7;
	// - One horiz. significant, no vertical significant, one or more
	// diagonal significant
	for (i = 1; i<16; i++) {
		ZC_LUT_LH[STATE_H_L_R1 | i] = 8;
		ZC_LUT_LH[STATE_H_R_R1 | i] = 8;
	}
	// - One horiz. significant, one or more vertical significant,
	// diagonal irrelevant
	for (i = 1; i<4; i++) {
		for (j = 0; j<16; j++) {
			ZC_LUT_LH[STATE_H_L_R1 | (i << 4) | j] = 9;
			ZC_LUT_LH[STATE_H_R_R1 | (i << 4) | j] = 9;
		}
	}
	// - Two horiz. significant, others irrelevant
	for (i = 0; i<64; i++) {
		ZC_LUT_LH[STATE_H_L_R1 | STATE_H_R_R1 | i] = 10;
	}

	// HL

	// - No neighbors significant
	ZC_LUT_HL[0] = 2;
	// - No horizontal or vertical neighbors significant
	for (i = 1; i<16; i++) { // Two or more diagonal coeffs significant
		ZC_LUT_HL[i] = 4;
	}
	for (i = 0; i<4; i++) { // Only one diagonal coeff significant
		ZC_LUT_HL[1 << i] = 3;
	}
	// - No vertical significant, diagonal irrelevant
	for (i = 0; i<16; i++) {
		// One horiz. significant
		ZC_LUT_HL[STATE_H_L_R1 | i] = 5;
		ZC_LUT_HL[STATE_H_R_R1 | i] = 5;
		// Two horiz. significant
		ZC_LUT_HL[STATE_H_L_R1 | STATE_H_R_R1 | i] = 6;
	}
	// - One vert. significant, diagonal/horizontal non-significant
	ZC_LUT_HL[STATE_V_U_R1] = 7;
	ZC_LUT_HL[STATE_V_D_R1] = 7;
	// - One vert. significant, horizontal non-significant, one or more
	// diag. significant
	for (i = 1; i<16; i++) {
		ZC_LUT_HL[STATE_V_U_R1 | i] = 8;
		ZC_LUT_HL[STATE_V_D_R1 | i] = 8;
	}
	// - One vertical significant, one or more horizontal significant,
	// diagonal irrelevant
	for (i = 1; i<4; i++) {
		for (j = 0; j<16; j++) {
			ZC_LUT_HL[(i << 6) | STATE_V_U_R1 | j] = 9;
			ZC_LUT_HL[(i << 6) | STATE_V_D_R1 | j] = 9;
		}
	}
	// - Two vertical significant, others irrelevant
	for (i = 0; i<4; i++) {
		for (j = 0; j<16; j++) {
			ZC_LUT_HL[(i << 6) | STATE_V_U_R1 | STATE_V_D_R1 | j] = 10;
		}
	}

	// HH
	int twoBits[6] = { 3,5,6,9,10,12 }; // Figures (between 0 and 15)
									   // countaning 2 and only 2 bits on in its binary representation.

	int oneBit[4] = { 1,2,4,8 }; // Figures (between 0 and 15)
								// countaning 1 and only 1 bit on in its binary representation.

	int twoLeast[11] = { 3,5,6,7,9,10,11,12,13,14,15 }; // Figures
													  // (between 0 and 15) countaining, at least, 2 bits on in its
													  // binary representation. 

	int threeLeast[5] = { 7,11,13,14,15 }; // Figures
										  // (between 0 and 15) countaining, at least, 3 bits on in its
										  // binary representation.

										  // - None significant
	ZC_LUT_HH[0] = 2;

	// - One horizontal+vertical significant, none diagonal
	for (i = 0; i<4; i++)
		ZC_LUT_HH[oneBit[i] << 4] = 3;

	// - Two or more horizontal+vertical significant, diagonal non-signif
	for (i = 0; i<11; i++)
		ZC_LUT_HH[twoLeast[i] << 4] = 4;

	// - One diagonal significant, horiz./vert. non-significant
	for (i = 0; i<4; i++)
		ZC_LUT_HH[oneBit[i]] = 5;

	// - One diagonal significant, one horiz.+vert. significant
	for (i = 0; i<4; i++)
		for (j = 0; j<4; j++)
			ZC_LUT_HH[(oneBit[i] << 4) | oneBit[j]] = 6;

	// - One diag signif, two or more horiz+vert signif
	for (i = 0; i<11; i++)
		for (j = 0; j<4; j++)
			ZC_LUT_HH[(twoLeast[i] << 4) | oneBit[j]] = 7;

	// - Two diagonal significant, none horiz+vert significant
	for (i = 0; i<11; i++)
		ZC_LUT_HH[twoBits[i]] = 8;

	// - Two diagonal significant, one or more horiz+vert significant
	for (j = 0; j<11; j++)
		for (i = 1; i<16; i++)
			ZC_LUT_HH[(i << 4) | twoBits[j]] = 9;

	// - Three or more diagonal significant, horiz+vert irrelevant
	for (i = 0; i<16; i++)
		for (j = 0; j<5; j++)
			ZC_LUT_HH[(i << 4) | threeLeast[j]] = 10;


	// Initialize the SC lookup tables

	// Use an intermediate sign code lookup table that is similar to the
	// one in the VM text, in that it depends on the 'h' and 'v'
	// quantities. The index into this table is a 6 bit index, the top 3
	// bits are (h+1) and the low 3 bits (v+1).
	inter_sc_lut = new int[19]; //19 or (36(?) - but probably they meant 38...)
	inter_sc_lut[(2 << 3) | 2] = 15;
	inter_sc_lut[(2 << 3) | 1] = 14;
	inter_sc_lut[(2 << 3) | 0] = 13;
	inter_sc_lut[(1 << 3) | 2] = 12;
	inter_sc_lut[(1 << 3) | 1] = 11;
	inter_sc_lut[(1 << 3) | 0] = 12 | INT_SIGN_BIT;
	inter_sc_lut[(0 << 3) | 2] = 13 | INT_SIGN_BIT;
	inter_sc_lut[(0 << 3) | 1] = 14 | INT_SIGN_BIT;
	inter_sc_lut[(0 << 3) | 0] = 15 | INT_SIGN_BIT;

	// Using the intermediate sign code lookup table create the final
	// one. The index into this table is a 9 bit index, the low 4 bits are 
	// the significance of the 4 horizontal/vertical neighbors, while the
	// top 4 bits are the signs of those neighbors. The bit in the middle
	// is ignored. This index arrangement matches the state bits in the
	// 'state' array, thus direct addressing of the table can be done from 
	// the sate information.
	for (i = 0; i<(1 << SC_LUT_BITS) - 1; i++) {
		ds = i & 0x01;        // significance of down neighbor
		us = (i >> 1) & 0x01; // significance of up neighbor
		rs = (i >> 2) & 0x01; // significance of right neighbor
		ls = (i >> 3) & 0x01; // significance of left neighbor
		dsgn = (i >> 5) & 0x01; // sign of down neighbor
		usgn = (i >> 6) & 0x01; // sign of up neighbor
		rsgn = (i >> 7) & 0x01; // sign of right neighbor
		lsgn = (i >> 8) & 0x01; // sign of left neighbor
								// Calculate 'h' and 'v' as in VM text
		h = ls*(1 - 2 * lsgn) + rs*(1 - 2 * rsgn);
		h = (h >= -1) ? h : -1;
		h = (h <= 1) ? h : 1;
		v = us*(1 - 2 * usgn) + ds*(1 - 2 * dsgn);
		v = (v >= -1) ? v : -1;
		v = (v <= 1) ? v : 1;
		// Get context and sign predictor from 'inter_sc_lut'
		SC_LUT[i] = inter_sc_lut[(h + 1) << 3 | (v + 1)];
	}

	// Initialize the MR lookup tables

	// None significant, prev MR off
	MR_LUT[0] = 16;
	// One or more significant, prev MR off
	for (i = 1; i<(1 << (MR_LUT_BITS - 1)); i++) {
		MR_LUT[i] = 17;
	}
	// Previous MR on, significance irrelevant
	for (; i<(1 << MR_LUT_BITS); i++) {
		MR_LUT[i] = 18;
	}
}


CodeBlock * EntropyDecoder::fillCodeBlock(Subband *sb, CodeBlock * cblk){
	int *zc_lut;     // The ZC lookup table to use
	int *out_data;   // The outupt data buffer
	int npasses;      // The number of coding passes to perform
	int curbp;        // The current magnitude bit-plane (starts at 30)
	boolean error;    // Error indicator
	int tslen;        // Length of first terminated segment
	int tsidx;        // Index of current terminated segment


	out_data = cblk->out_data;


	// Set data values to 0
	cblk->initDataSet();

	// Get the length of the first terminated segment
	tslen = cblk->cbLen;
	tsidx = 0;
	// Initialize for decoding
	npasses = cblk->truncationPointsAmount;
	mq = new MQDecoder(src, NUM_CTXTS, MQ_INIT);
	// We always start by an MQ segment
	mq->nextSegment();
	mq->resetCtxts();
	error = false;
	/** The ID for the LL orientation */
	const int WT_ORIENT_LL = 0;

	/** The ID for the HL (horizontal high-pass) orientation */
	const int WT_ORIENT_HL = 1;

	/** The ID for the LH (vertical high-pass) orientation */
	const int WT_ORIENT_LH = 2;

	/** The ID for the HH orientation */
	const int WT_ORIENT_HH = 3;

	// Choose correct ZC lookup table for global orientation
	switch (sb->gOrient) {
		case WT_ORIENT_HL:
			zc_lut = ZC_LUT_HL;
			break;
		case WT_ORIENT_LH:
		case WT_ORIENT_LL:
			zc_lut = ZC_LUT_LH;
			break;
		case WT_ORIENT_HH:
			zc_lut = ZC_LUT_HH;
			break;
	}


	// Loop on bit-planes and passes

	curbp = 30 - cblk->msbSkipped;
	//to - do the rest
	return cblk;
}



bool EntropyDecoder::cleanuppass() {
	//to-do
	return true;
}

bool EntropyDecoder::sigProgPass() {
	//to-do 
	return true;
}

bool EntropyDecoder::rawSigProgPass() {
	//to-do
	return true;
}

bool EntropyDecoder::magRefPass() {
	//to-do
	return true;
}

bool EntropyDecoder::rawMagRefPass() {
	//to-do
	return true;
}