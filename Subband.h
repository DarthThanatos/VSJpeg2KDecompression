#pragma once
#include "common.h"
#include <string>

/** The ID for the LL orientation */
const int WT_ORIENT_LL = 0;

/** The ID for the HL (horizontal high-pass) orientation */
const int WT_ORIENT_HL = 1;

/** The ID for the LH (vertical high-pass) orientation */
const int WT_ORIENT_LH = 2;

/** The ID for the HH orientation */
const int WT_ORIENT_HH = 3;

class Subband {
	public:
		Subband(int, int, int, int, int, int, int);
		Subband *getSubbandAt(int, int);
		void printCoefficients(int* coeffs);

		int w, h, oX, oY, totalDecompLvls, rLvl, type;
		Subband *LL, *HL, *LH, *HH;
		Subband **allChildren;
		int childrenCount = 0;

		string toString();
		boolean isNode;
};