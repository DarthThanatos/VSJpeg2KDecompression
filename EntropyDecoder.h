#pragma once
#include "common.h"

class EntropyDecoder {
public :
	EntropyDecoder(StreamReader* src);

private: 

	/** The bit based input for arithmetic coding bypass (i.e. raw) coding */
	StreamReader* src;

	/** The MQ decoder to use. It has in as the underlying source of coded
	* data. */
	MQDecoder *mq;

	/** Number of bits used for the Zero Coding lookup table */
    int ZC_LUT_BITS = 8;

	/** Zero Coding context lookup tables for the LH global orientation */
	int *ZC_LUT_LH = new int[1 << ZC_LUT_BITS];

	/** Zero Coding context lookup tables for the HL global orientation */
	int *ZC_LUT_HL = new int[1 << ZC_LUT_BITS];

	/** Zero Coding context lookup tables for the HH global orientation */
	int *ZC_LUT_HH = new int[1 << ZC_LUT_BITS];

	/** Number of bits used for the Sign Coding lookup table */
	int SC_LUT_BITS = 9;

	/** Sign Coding context lookup table. The index into the table is a 9 bit
	* index, which correspond the the value in the 'state' array shifted by
	* 'SC_SHIFT'. Bits 8-5 are the signs of the horizontal-left,
	* horizontal-right, vertical-up and vertical-down neighbors,
	* respectively. Bit 4 is not used (0 or 1 makes no difference). Bits 3-0
	* are the significance of the horizontal-left, horizontal-right,
	* vertical-up and vertical-down neighbors, respectively. The least 4 bits
	* of the value in the lookup table define the context number and the sign
	* bit defines the "sign predictor". */
	int *SC_LUT = new int[1 << SC_LUT_BITS];

	/** The mask to obtain the context index from the 'SC_LUT' */
	int SC_LUT_MASK = (1 << 4) - 1;

	/** The shift to obtain the sign predictor from the 'SC_LUT'. It must be
	* an unsigned shift. */
	 int SC_SPRED_SHIFT = 31;

	/** The sign bit for int data */
	int INT_SIGN_BIT = 1 << 31;

	/** The number of bits used for the Magnitude Refinement lookup table */
	int MR_LUT_BITS = 9;

	/** Magnitude Refinement context lookup table */
	int *MR_LUT = new int[1 << MR_LUT_BITS];

	/** The number of contexts used */
	int NUM_CTXTS = 19;

	/** The RLC context */
	int RLC_CTXT = 1;

	/** The UNIFORM context (with a uniform probability distribution which
	* does not adapt) */
	int UNIF_CTXT = 0;

	/** The initial states for the MQ coder */
	int MQ_INIT[19] = { 46, 3, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	/** The 4 symbol segmentation marker (decimal 10, which is binary sequence
	1010) */
	int SEG_MARKER = 10;

	/**
	* The state array for entropy coding. Each element of the state array
	* stores the state of two coefficients. The lower 16 bits store the state
	* of a coefficient in row 'i' and column 'j', while the upper 16 bits
	* store the state of a coefficient in row 'i+1' and column 'j'. The 'i'
	* row is either the first or the third row of a stripe. This packing of
	* the states into 32 bit words allows a faster scan of all coefficients
	* on each coding pass and diminished the amount of data transferred. The
	* size of the state array is increased by 1 on each side (top, bottom,
	* left, right) to handle boundary conditions without any special logic.
	*
	* <P>The state of a coefficient is stored in the following way in the
	* lower 16 bits, where bit 0 is the least significant bit. Bit 15 is the
	* significance of a coefficient (0 if non-significant, 1 otherwise). Bit
	* 14 is the visited state (i.e. if a coefficient has been coded in the
	* significance propagation pass of the current bit-plane). Bit 13 is the
	* "non zero-context" state (i.e. if one of the eight immediate neighbors
	* is significant it is 1, otherwise is 0). Bits 12 to 9 store the sign of
	* the already significant left, right, up and down neighbors (1 for
	* negative, 0 for positive or not yet significant). Bit 8 indicates if
	* the magnitude refinement has already been applied to the
	* coefficient. Bits 7 to 4 store the significance of the left, right, up
	* and down neighbors (1 for significant, 0 for non significant). Bits 3
	* to 0 store the significance of the diagonal coefficients (up-left,
	* up-right, down-left and down-right; 1 for significant, 0 for non
	* significant).
	*
	* <P>The upper 16 bits the state is stored as in the lower 16 bits,
	* but with the bits shifted up by 16.
	*
	* <P>The lower 16 bits are referred to as "row 1" ("R1") while the upper
	* 16 bits are referred to as "row 2" ("R2").
	* */
	int *state;

	/** The separation between the upper and lower bits in the state array: 16
	* */
	int STATE_SEP = 16;

	/** The flag bit for the significance in the state array, for row 1. */
    int STATE_SIG_R1 = 1 << 15;

	/** The flag bit for the "visited" bit in the state array, for row 1. */
	int STATE_VISITED_R1 = 1 << 14;

	/** The flag bit for the "not zero context" bit in the state array, for
	* row 1. This bit is always the OR of bits STATE_H_L_R1, STATE_H_R_R1,
	* STATE_V_U_R1, STATE_V_D_R1, STATE_D_UL_R1, STATE_D_UR_R1, STATE_D_DL_R1
	* and STATE_D_DR_R1. */
	int STATE_NZ_CTXT_R1 = 1 << 13;

	/** The flag bit for the horizontal-left sign in the state array, for row
	* 1. This bit can only be set if the STATE_H_L_R1 is also set. */
	int STATE_H_L_SIGN_R1 = 1 << 12;

	/** The flag bit for the horizontal-right sign in the state array, for
	* row 1. This bit can only be set if the STATE_H_R_R1 is also set. */
	int STATE_H_R_SIGN_R1 = 1 << 11;

	/** The flag bit for the vertical-up sign in the state array, for row
	* 1. This bit can only be set if the STATE_V_U_R1 is also set. */
	int STATE_V_U_SIGN_R1 = 1 << 10;

	/** The flag bit for the vertical-down sign in the state array, for row
	* 1. This bit can only be set if the STATE_V_D_R1 is also set. */
	int STATE_V_D_SIGN_R1 = 1 << 9;

	/** The flag bit for the previous MR primitive applied in the state array,
	for row 1. */
	int STATE_PREV_MR_R1 = 1 << 8;

	/** The flag bit for the horizontal-left significance in the state array,
	for row 1. */
	int STATE_H_L_R1 = 1 << 7;

	/** The flag bit for the horizontal-right significance in the state array,
	for row 1. */
	int STATE_H_R_R1 = 1 << 6;

	/** The flag bit for the vertical-up significance in the state array, for
	row 1.  */
	int STATE_V_U_R1 = 1 << 5;

	/** The flag bit for the vertical-down significance in the state array,
	for row 1.  */
	int STATE_V_D_R1 = 1 << 4;

	/** The flag bit for the diagonal up-left significance in the state array,
	for row 1. */
	int STATE_D_UL_R1 = 1 << 3;

	/** The flag bit for the diagonal up-right significance in the state
	array, for row 1.*/
	int STATE_D_UR_R1 = 1 << 2;

	/** The flag bit for the diagonal down-left significance in the state
	array, for row 1. */
	int STATE_D_DL_R1 = 1 << 1;

	/** The flag bit for the diagonal down-right significance in the state
	array , for row 1.*/
	int STATE_D_DR_R1 = 1;

	/** The flag bit for the significance in the state array, for row 2. */
	int STATE_SIG_R2 = STATE_SIG_R1 << STATE_SEP;

	/** The flag bit for the "visited" bit in the state array, for row 2. */
	int STATE_VISITED_R2 = STATE_VISITED_R1 << STATE_SEP;

	/** The flag bit for the "not zero context" bit in the state array, for
	* row 2. This bit is always the OR of bits STATE_H_L_R2, STATE_H_R_R2,
	* STATE_V_U_R2, STATE_V_D_R2, STATE_D_UL_R2, STATE_D_UR_R2, STATE_D_DL_R2
	* and STATE_D_DR_R2. */
	int STATE_NZ_CTXT_R2 = STATE_NZ_CTXT_R1 << STATE_SEP;

	/** The flag bit for the horizontal-left sign in the state array, for row
	* 2. This bit can only be set if the STATE_H_L_R2 is also set. */
	int STATE_H_L_SIGN_R2 = STATE_H_L_SIGN_R1 << STATE_SEP;

	/** The flag bit for the horizontal-right sign in the state array, for
	* row 2. This bit can only be set if the STATE_H_R_R2 is also set. */
	int STATE_H_R_SIGN_R2 = STATE_H_R_SIGN_R1 << STATE_SEP;

	/** The flag bit for the vertical-up sign in the state array, for row
	* 2. This bit can only be set if the STATE_V_U_R2 is also set. */
	int STATE_V_U_SIGN_R2 = STATE_V_U_SIGN_R1 << STATE_SEP;

	/** The flag bit for the vertical-down sign in the state array, for row
	* 2. This bit can only be set if the STATE_V_D_R2 is also set. */
	int STATE_V_D_SIGN_R2 = STATE_V_D_SIGN_R1 << STATE_SEP;

	/** The flag bit for the previous MR primitive applied in the state array,
	for row 2. */
	int STATE_PREV_MR_R2 = STATE_PREV_MR_R1 << STATE_SEP;

	/** The flag bit for the horizontal-left significance in the state array,
	for row 2. */
	int STATE_H_L_R2 = STATE_H_L_R1 << STATE_SEP;

	/** The flag bit for the horizontal-right significance in the state array,
	for row 2. */
	int STATE_H_R_R2 = STATE_H_R_R1 << STATE_SEP;

	/** The flag bit for the vertical-up significance in the state array, for
	row 2.  */
	int STATE_V_U_R2 = STATE_V_U_R1 << STATE_SEP;

	/** The flag bit for the vertical-down significance in the state array,
	for row 2.  */
	int STATE_V_D_R2 = STATE_V_D_R1 << STATE_SEP;

	/** The flag bit for the diagonal up-left significance in the state array,
	for row 2. */
	int STATE_D_UL_R2 = STATE_D_UL_R1 << STATE_SEP;

	/** The flag bit for the diagonal up-right significance in the state
	array, for row 2.*/
	int STATE_D_UR_R2 = STATE_D_UR_R1 << STATE_SEP;

	/** The flag bit for the diagonal down-left significance in the state
	array, for row 2. */
	int STATE_D_DL_R2 = STATE_D_DL_R1 << STATE_SEP;

	/** The flag bit for the diagonal down-right significance in the state
	array , for row 2.*/
	int STATE_D_DR_R2 = STATE_D_DR_R1 << STATE_SEP;

	/** The mask to isolate the significance bits for row 1 and 2 of the state
	* array. */
	int SIG_MASK_R1R2 = STATE_SIG_R1 | STATE_SIG_R2;

	/** The mask to isolate the visited bits for row 1 and 2 of the state
	* array. */
	int VSTD_MASK_R1R2 = STATE_VISITED_R1 | STATE_VISITED_R2;

	/** The mask to isolate the bits necessary to identify RLC coding state
	* (significant, visited and non-zero context, for row 1 and 2). */
	int RLC_MASK_R1R2 =
		STATE_SIG_R1 | STATE_SIG_R2 |
		STATE_VISITED_R1 | STATE_VISITED_R2 |
		STATE_NZ_CTXT_R1 | STATE_NZ_CTXT_R2;

	/** The mask to obtain the ZC_LUT index from the 'state' information */
	// This is needed because of the STATE_V_D_SIGN, STATE_V_U_SIGN,
	// STATE_H_R_SIGN, and STATE_H_L_SIGN bits.
	int ZC_MASK = (1 << 8) - 1;

	/** The shift to obtain the SC index to 'SC_LUT' from the 'state'
	* information, for row 1. */
	int SC_SHIFT_R1 = 4;

	/** The shift to obtain the SC index to 'SC_LUT' from the state
	* information, for row 2. */
	int SC_SHIFT_R2 = SC_SHIFT_R1 + STATE_SEP;

	/** The bit mask to isolate the state bits relative to the sign coding
	* lookup table ('SC_LUT'). */
	int SC_MASK = (1 << SC_LUT_BITS) - 1;

	/** The mask to obtain the MR index to 'MR_LUT' from the 'state'
	* information. It is to be applied after the 'MR_SHIFT' */
	int MR_MASK = (1 << 9) - 1;

	CodeBlock *fillCodeBlock( Subband *sb,	CodeBlock * cblk);
	bool sigProgPass();
	bool rawSigProgPass();
	bool magRefPass(); 
	bool rawMagRefPass(); 
	bool cleanuppass();

};
