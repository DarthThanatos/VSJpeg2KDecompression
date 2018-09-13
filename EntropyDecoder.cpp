#include "common.h"

EntropyDecoder::EntropyDecoder(MetadataReader *mr) {
	int i, j;
	double val, deltaMSE;
	int *inter_sc_lut;
	int ds, us, rs, ls;
	int dsgn, usgn, rsgn, lsgn;
	int h, v;

	stateLength = (mr->codeblock_width + 2) *	((mr->codeblock_height + 1) / 2 + 2);
	state = new int[stateLength];
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
	for (i = 0; i < 6; i++) {
		ZC_LUT_HH[twoBits[i]] = 8;
	}

	// - Two diagonal significant, one or more horiz+vert significant
	for (j = 0; j<6; j++)
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

void initArrays(CodeBlock *cblk, int* state, int stateLength) {
	for (int i = 0; i < stateLength; i++) state[i] = 0;
	cblk->coefficients = new int[cblk->w * cblk->h];
	for (int i = 0; i < cblk->w * cblk->h; i++) cblk->coefficients[i] = 0;
}

void EntropyDecoder::decode(CodeBlock ****cblks) {
	for (int c = 0; c < ALL_C; c++) {
		for (int r = 0; r < ALL_R; r++) {
			int mins = (r == 0) ? 0 : 1;
			int maxs = (r == 0) ? 1 : 4;
			for (int s = mins; s < maxs; s++) {
				CodeBlock *cblk = cblks[c][r][s];
				initArrays(cblk, state, stateLength);
				decodeCodeBlock(cblk, s);
			}			
		}
	}
}

void EntropyDecoder::decodeCodeBlock(CodeBlock * cblk, int subbandType){
	int *zc_lut;     // The ZC lookup table to use
	int npasses;      // The number of coding passes to perform
	int curbp;        // The current magnitude bit-plane (starts at 30)

	npasses = cblk->ntp;
	mq = new MQDecoder(cblk->data, NUM_CTXTS, MQ_INIT);

	// Choose correct ZC lookup table for global orientation
	switch (subbandType) {
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
		default: return;
	}

	curbp = 30 - cblk->msbSkipped;
	if (curbp >= 0 && npasses > 0) {
		cleanuppass(cblk, curbp, state, zc_lut);
		npasses--;
		curbp--;
	}

	while (curbp >= 0 && npasses > 0) {
		sigProgPass(cblk, curbp, state, zc_lut);
		npasses--;
		if (npasses <= 0) break;
		magRefPass(cblk, curbp, state, zc_lut);
		npasses--;
		if (npasses <= 0) break;
		cleanuppass(cblk, curbp, state, zc_lut);
		npasses--;
		curbp--;
	}
}


int EntropyDecoder::getSym(int data[], int k, int csj, int setmask, int sc_shift) {
	// Sample that became significant is first row of its column half
	unsigned int ctxt = SC_LUT[(csj >> sc_shift)&SC_MASK];
	unsigned int xorBit = ctxt >> SC_SPRED_SHIFT;
	int sym = mq->decodeSymbol(ctxt & SC_LUT_MASK) ^ (xorBit);
	data[k] = (sym << 31) | setmask;
	return sym;
}

int EntropyDecoder::updateFirstRowNeighbours(int state[], int j, int off_ul, int off_ur, int csj, int sym, int sscanw) {
	// Update state information (significant bit,
	// visited bit, neighbor significant bit of
	// neighbors, non zero context of neighbors, sign
	// of neighbors)
	state[j + off_ul] |= STATE_NZ_CTXT_R2 | STATE_D_DR_R2;
	state[j + off_ur] |= STATE_NZ_CTXT_R2 | STATE_D_DL_R2;
	// Update sign state information of neighbors
	csj |= STATE_SIG_R1 | STATE_NZ_CTXT_R2 | STATE_V_U_R2 | STATE_VISITED_R1;
	state[j - sscanw] |= STATE_NZ_CTXT_R2 | STATE_V_D_R2;
	state[j + 1] |= STATE_NZ_CTXT_R1 | STATE_NZ_CTXT_R2 | STATE_H_L_R1 | STATE_D_UL_R2;
	state[j - 1] |= STATE_NZ_CTXT_R1 | STATE_NZ_CTXT_R2 | STATE_H_R_R1 | STATE_D_UR_R2;
	if (sym != 0) {
		csj |= STATE_V_U_SIGN_R2;
		state[j - sscanw] |= STATE_V_D_SIGN_R2;
		state[j + 1] |= STATE_H_L_SIGN_R1;
		state[j - 1] |= STATE_H_R_SIGN_R1;
	}
	return csj;
}

int EntropyDecoder::updateSecondRowNeighbours(int state[], int j, int off_dl, int off_dr, int csj, int sym, int sscanw, boolean visitedToSet) {
	// Update state information (significant bit,
	// neighbor significant bit of neighbors, non zero
	// context of neighbors, sign of neighbors)
	state[j + off_dl] |= STATE_NZ_CTXT_R1 | STATE_D_UR_R1;
	state[j + off_dr] |= STATE_NZ_CTXT_R1 | STATE_D_UL_R1;
	csj |= STATE_SIG_R2 | STATE_NZ_CTXT_R1 | STATE_V_D_R1;
	state[j + sscanw] |= STATE_NZ_CTXT_R1 | STATE_V_U_R1;
	state[j + 1] |= STATE_NZ_CTXT_R1 | STATE_NZ_CTXT_R2 | STATE_D_DL_R1 | STATE_H_L_R2;
	state[j - 1] |= STATE_NZ_CTXT_R1 | STATE_NZ_CTXT_R2 | STATE_D_DR_R1 | STATE_H_R_R2;
	// Update sign state information of neighbors
	if (sym != 0) {
		csj |= STATE_V_D_SIGN_R1;
		state[j + sscanw] |= STATE_V_U_SIGN_R1;
		state[j + 1] |= STATE_H_L_SIGN_R2;
		state[j - 1] |= STATE_H_R_SIGN_R2;
	}
	if (visitedToSet) {
		csj |= STATE_VISITED_R2;
	}
	return csj;
}

void EntropyDecoder::notSigNotVis(int data[], int k, int csj, int zc_lut[], int setmask, int sscanw, int dscanw, int j, int off_ul, int off_ur, int off_dl, int off_dr, int sheight, int sheightTresh) {
	if ((csj & VSTD_MASK_R1R2) != VSTD_MASK_R1R2) {
		// Scan first row
		if ((csj & (STATE_SIG_R1 | STATE_VISITED_R1)) == 0) {
			// Use zero coding
			if (mq->decodeSymbol(zc_lut[csj&ZC_MASK]) != 0) {
				int sym = getSym(data, k, csj, setmask, SC_SHIFT_R1);
				csj = updateFirstRowNeighbours(state, j, off_ul, off_ur, csj, sym, sscanw);
			}
		}
		if (sheight < sheightTresh) {
			csj &= ~(STATE_VISITED_R1 | STATE_VISITED_R2);
			state[j] = csj;
			return;
		}
		// Scan second row
		if ((csj & (STATE_SIG_R2 | STATE_VISITED_R2)) == 0) {
			k += dscanw;
			// Use zero coding
			unsigned int csj_uns = csj;
			unsigned int lut_index = csj_uns >> STATE_SEP;
			if (mq->decodeSymbol(zc_lut[(lut_index)& ZC_MASK]) != 0) {
				int sym = getSym(data, k, csj, setmask, SC_SHIFT_R2);
				csj = updateSecondRowNeighbours(state, j, off_dl, off_dr, csj, sym, sscanw, true);
			}
		}
	}
	csj &= ~(STATE_VISITED_R1 | STATE_VISITED_R2);
	state[j] = csj;
}

void EntropyDecoder::cleanuppass(CodeBlock *cblk, int bp, int* state, int *zc_lut) {
	int j, sj;        // The state index for line and stripe
	int k, sk;        // The data index for line and stripe
	int dscanw;      // The data scan-width
	int sscanw;      // The state scan-width
	int jstep;       // Stripe to stripe step for 'sj'
	int kstep;       // Stripe to stripe step for 'sk'
	int stopsk;      // The loop limit on the variable sk
	int csj;         // Local copy (i.e. cached) of 'state[j]'
	int setmask;     // The mask to set current and lower bit-planes to 1/2 approximation
	int sym;         // The decoded symbol
	int rlclen;      // Length of RLC
	int *data;      // The data buffer
	int s;           // The stripe index
	int nstripes;    // The number of stripes in the code-block
	int sheight;     // Height of the current stripe
	int off_ul, off_ur, off_dr, off_dl; // offsets
	dscanw = cblk->w;
	sscanw = cblk->w + 2;
	jstep = sscanw * STRIPE_HEIGHT / 2 - cblk->w;
	kstep = dscanw * STRIPE_HEIGHT - cblk->w;
	setmask = (3 << bp) >> 1;
	data = cblk->coefficients;
	nstripes = (cblk->h + STRIPE_HEIGHT - 1) / STRIPE_HEIGHT;

	// Pre-calculate offsets in 'state' for diagonal neighbors
	off_ul = -sscanw - 1;  // up-left
	off_ur = -sscanw + 1; // up-right
	off_dr = sscanw + 1;   // down-right
	off_dl = sscanw - 1;   // down-left

	// Decode stripe by stripe
	sk = 0;
	sj = sscanw + 1;
	for (s = nstripes - 1; s >= 0; s--, sk += kstep, sj += jstep) {
		sheight = (s != 0) ? STRIPE_HEIGHT : cblk->h - (nstripes - 1)*STRIPE_HEIGHT;
		stopsk = sk + cblk->w;
		// Scan by set of 1 stripe column at a time
		for (; sk < stopsk; sk++, sj++) {
			// Start column
			j = sj;
			csj = state[j];
		top_half:
			{
				// Check for RLC: if all samples are not significant, not
				// visited and do not have a non-zero context, and column
				// is full height, we do RLC.
				if (csj == 0 && state[j + sscanw] == 0 && sheight == STRIPE_HEIGHT) {
					if (mq->decodeSymbol(RLC_CTXT) != 0) {
						// run-length is significant, decode length
						rlclen = mq->decodeSymbol(UNIF_CTXT) << 1;
						rlclen |= mq->decodeSymbol(UNIF_CTXT);
						// Set 'k' and 'j' accordingly
						k = sk + rlclen * dscanw;
						if (rlclen > 1) {
							j += sscanw;
							csj = state[j];
						}
					}
					else { // RLC is insignificant
						   // Goto next column
						continue;
					}
					// We just decoded the length of a significant RLC
					// and a sample became significant
					// Use sign coding
					if ((rlclen & 0x01) == 0) {
						sym = getSym(data, k, csj, setmask, SC_SHIFT_R1);
						csj = updateFirstRowNeighbours(state, j, off_ul, off_ur, csj, sym, sscanw);
						// Changes to csj are saved later
						if ((rlclen >> 1) == 1) { //10
							// Sample that became significant is in
							// bottom half of column => jump to bottom
							// half
							goto bottom_half;
						}
						// Otherwise sample that became significant is in
						// top half of column => continue on top half
					}
					else {
						sym = getSym(data, k, csj, setmask, SC_SHIFT_R2);
						csj = updateSecondRowNeighbours(state, j, off_dl, off_dr, csj, sym, sscanw, false);
						// Save changes to csj
						state[j] = csj;
						if ((rlclen >> 1) == 1) { //11
							// Sample that became significant is in bottom
							// half of column => we're done with this
							// column
							continue;
						}
						// 01
						// Otherwise sample that became significant is in
						// top half of column => we're done with top
						// column
						j += sscanw;
						csj = state[j];
						goto bottom_half;
					}
				} // 00
				notSigNotVis(data, sk, csj, zc_lut, setmask, sscanw, dscanw, j, off_ul, off_ur, off_dl, off_dr, sheight, 2);
				// Do half bottom of column
				if (sheight < 3) continue;
				j += sscanw;
				csj = state[j];
			} // end of 'top_half' block
		bottom_half:
			notSigNotVis(data, sk + (dscanw * 2), csj, zc_lut, setmask, sscanw, dscanw, j, off_ul, off_ur, off_dl, off_dr, sheight, 4);
		}
	}
}

void EntropyDecoder::sigProgPass(CodeBlock *cblk, int bp, int* state, int *zc_lut) {
	int j, sj;        // The state index for line and stripe
	int k, sk;        // The data index for line and stripe
	int dscanw;      // The data scan-width
	int sscanw;      // The state scan-width
	int jstep;       // Stripe to stripe step for 'sj'
	int kstep;       // Stripe to stripe step for 'sk'
	int stopsk;      // The loop limit on the variable sk
	int csj;         // Local copy (i.e. cached) of 'state[j]'
	int setmask;     // The mask to set current and lower bit-planes to 1/2
					 // approximation
	int sym;         // The symbol to code
	int *data;      // The data buffer
	int s;           // The stripe index
	int nstripes;    // The number of stripes in the code-block
	int sheight;     // Height of the current stripe
	int off_ul, off_ur, off_dr, off_dl; // offsets

	dscanw = cblk->w;
	sscanw = cblk->w + 2;
	jstep = sscanw * STRIPE_HEIGHT / 2 - cblk->w;
	kstep = dscanw * STRIPE_HEIGHT - cblk->w;
	setmask = (3 << bp) >> 1;
	data = cblk->coefficients;
	nstripes = (cblk->h + STRIPE_HEIGHT - 1) / STRIPE_HEIGHT;

	// Pre-calculate offsets in 'state' for diagonal neighbors
	off_ul = -sscanw - 1;  // up-left
	off_ur = -sscanw + 1; // up-right
	off_dr = sscanw + 1;   // down-right
	off_dl = sscanw - 1;   // down-left

	// Decode stripe by stripe
	sk = 0;
	sj = sscanw + 1;
	for (s = nstripes - 1; s >= 0; s--, sk += kstep, sj += jstep) {
		sheight = (s != 0) ? STRIPE_HEIGHT : cblk->h - (nstripes - 1)*STRIPE_HEIGHT;
		stopsk = sk + cblk->w;
		// Scan by set of 1 stripe column at a time
		for (; sk < stopsk; sk++, sj++) {
			// Do half top of column
			j = sj;
			csj = state[j];
			// If any of the two samples is not significant and has a
			// non-zero context (i.e. some neighbor is significant) we can 
			// not skip them
			if ((((~csj)) & SIG_MASK_R1R2) != 0) {
				k = sk;
				// Scan first row
				if ((csj & (STATE_SIG_R1 | STATE_NZ_CTXT_R1)) == STATE_NZ_CTXT_R1) {
					// Use zero coding
					if (mq->decodeSymbol(zc_lut[csj&ZC_MASK]) != 0) {
						sym = getSym(data, k, csj, setmask, SC_SHIFT_R1);
						csj = updateFirstRowNeighbours(state, j, off_ul, off_ur, csj, sym, sscanw);
					}
					else {
						csj |= STATE_VISITED_R1;
					}
				}
				state[j] = csj;
				if (sheight < 2) {
					continue;
				}
				// Scan second row
				if ((csj & (STATE_SIG_R2 | STATE_NZ_CTXT_R2)) == STATE_NZ_CTXT_R2) {
					k += dscanw;
					// Use zero coding
					unsigned int csj_uns = csj;
					unsigned int lut_index = csj_uns >> STATE_SEP;
					if (mq->decodeSymbol(zc_lut[(lut_index)&ZC_MASK]) != 0) {
						sym = getSym(data, k, csj, setmask, SC_SHIFT_R2);
						csj = updateSecondRowNeighbours(state, j, off_dl, off_dr, csj, sym, sscanw, true);
					}
					else {
						csj |= STATE_VISITED_R2;
					}
				}
				state[j] = csj;
			}
			// Do half bottom of column
			if (sheight < 3) continue;
			j += sscanw;
			csj = state[j];
			// If any of the two samples is not significant and has a
			// non-zero context (i.e. some neighbor is significant) we can 
			// not skip them
			if ((((~csj)) & SIG_MASK_R1R2) != 0) {
				k = sk + (dscanw * 2);
				// Scan first row
				if ((csj & (STATE_SIG_R1 | STATE_NZ_CTXT_R1)) == STATE_NZ_CTXT_R1) {
					// Use zero coding
					if (mq->decodeSymbol(zc_lut[csj&ZC_MASK]) != 0) {
						sym = getSym(data, k, csj, setmask, SC_SHIFT_R1);
						csj = updateFirstRowNeighbours(state, j, off_ul, off_ur, csj, sym, sscanw);
					}
					else {
						csj |= STATE_VISITED_R1;
					}
				}
				if (sheight < 4) {
					state[j] = csj;
					continue;
				}
				// Scan second row
				if ((csj & (STATE_SIG_R2 | STATE_NZ_CTXT_R2)) == STATE_NZ_CTXT_R2) {
					k += dscanw;
					// Use zero coding
					unsigned int csj_uns = csj;
					unsigned int lut_index = csj_uns >> STATE_SEP;
					if (mq->decodeSymbol(zc_lut[(lut_index)&  ZC_MASK]) != 0) {
						sym = getSym(data, k, csj, setmask, SC_SHIFT_R2);
						csj = updateSecondRowNeighbours(state, j, off_dl, off_dr, csj, sym, sscanw, true);
					}
					else {
						csj |= STATE_VISITED_R2;
					}
				}
				state[j] = csj;
			}
		}
	}
}


void EntropyDecoder::magRefPass(CodeBlock *cblk, int bp, int* state, int *zc_lut) {
	int j, sj;        // The state index for line and stripe
	int k, sk;        // The data index for line and stripe
	int dscanw;      // The data scan-width
	int sscanw;      // The state scan-width
	int jstep;       // Stripe to stripe step for 'sj'
	int kstep;       // Stripe to stripe step for 'sk'
	int stopsk;      // The loop limit on the variable sk
	int csj;         // Local copy (i.e. cached) of 'state[j]'
	int setmask; // The mask to set lower bit-planes to 1/2 approximation
	int resetmask;   // The mask to reset approximation bit-planes
	int sym;         // The symbol to decode
	int *data;      // The data buffer
	int s;           // The stripe index
	int nstripes;    // The number of stripes in the code-block
	int sheight;     // Height of the current stripe

	dscanw = cblk->w;
	sscanw = cblk->w + 2;
	jstep = sscanw * STRIPE_HEIGHT / 2 - cblk->w;
	kstep = dscanw * STRIPE_HEIGHT - cblk->w;
	setmask = (1 << bp) >> 1;
	resetmask = (-1) << (bp + 1);

	data = cblk->coefficients;
	nstripes = (cblk->h + STRIPE_HEIGHT - 1) / STRIPE_HEIGHT;

	// Decode stripe by stripe
	sk = 0;
	sj = sscanw + 1;
	for (s = nstripes - 1; s >= 0; s--, sk += kstep, sj += jstep) {
		sheight = (s != 0) ? STRIPE_HEIGHT : cblk->h - (nstripes - 1)*STRIPE_HEIGHT;
		stopsk = sk + cblk->w;
		// Scan by set of 1 stripe column at a time
		for (; sk < stopsk; sk++, sj++) {
			// Do half top of column
			j = sj;
			csj = state[j];
			// If any of the two samples is significant and not yet
			// visited in the current bit-plane we cannot skip them
			if ((((~csj)) & VSTD_MASK_R1R2) != 0) {
				k = sk;
				// Scan first row
				if ((csj & (STATE_SIG_R1 | STATE_VISITED_R1)) == STATE_SIG_R1) {
					sym = mq->decodeSymbol(MR_LUT[csj&MR_MASK]);
					data[k] &= resetmask;
					data[k] |= (sym << bp) | setmask;
					csj |= STATE_PREV_MR_R1;
				}
				if (sheight < 2) {
					state[j] = csj;
					continue;
				}
				// Scan second row
				if ((csj & (STATE_SIG_R2 | STATE_VISITED_R2)) ==
					STATE_SIG_R2) {
					k += dscanw;
					unsigned int csj_uns = csj;
					unsigned int lut_index = csj_uns >> STATE_SEP;
					sym = mq->decodeSymbol(MR_LUT[(lut_index)& MR_MASK]);
					data[k] &= resetmask;
					data[k] |= (sym << bp) | setmask;
					csj |= STATE_PREV_MR_R2;
				}
				state[j] = csj;
			}
			// Do half bottom of column
			if (sheight < 3) continue;
			j += sscanw;
			csj = state[j];
			// If any of the two samples is significant and not yet
			// visited in the current bit-plane we can not skip them
			if ((((~csj)) & VSTD_MASK_R1R2) != 0) {
				k = sk + (dscanw * 2);
				// Scan first row
				if ((csj & (STATE_SIG_R1 | STATE_VISITED_R1)) == STATE_SIG_R1) {
					sym = mq->decodeSymbol(MR_LUT[csj&MR_MASK]);
					data[k] &= resetmask;
					data[k] |= (sym << bp) | setmask;
					csj |= STATE_PREV_MR_R1;
				}
				if (sheight < 4) {
					state[j] = csj;
					continue;
				}
				// Scan second row
				if ((state[j] & (STATE_SIG_R2 | STATE_VISITED_R2)) == STATE_SIG_R2) {
					k += dscanw;
					unsigned int csj_uns = csj;
					unsigned int lut_index = csj_uns >> STATE_SEP;
					sym = mq->decodeSymbol(MR_LUT[(lut_index) & MR_MASK]);
					data[k] &= resetmask;
					data[k] |= (sym << bp) | setmask;
					csj |= STATE_PREV_MR_R2;
				}
				state[j] = csj;
			}
		}
	}
}