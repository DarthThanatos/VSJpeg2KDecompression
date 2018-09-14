#include "common.h"


InverseWaveletTransform::InverseWaveletTransform(CodeBlock ****cblks, MetadataReader *mr) {
	this->componentRoots = mr->componentsSubbandRoots;
	this->cblks = cblks;
	this->mr = mr;
}

int **InverseWaveletTransform::inverseSubbandCodeblocks() {
	int **reconstructedComponents = new int*[ALL_C];
	for (int c = 0; c < ALL_C; c++) {
		reconstructedComponents[c] = reconstructLvlRecursive(componentRoots[c], cblks[c]);
	}
	return reconstructedComponents;
}

void InverseWaveletTransform::leaf_(Subband *subband, CodeBlock ***c_codeblocks, int *subbandImg) {
	CodeBlock *cblk = c_codeblocks[subband->rLvl][subband->type];
	for (int i = 0; i < cblk->w * cblk->h; i++) {
		subbandImg[i] = cblk->coefficients[i];
	}

}

int *InverseWaveletTransform::reconstructLvlRecursive(Subband *subband, CodeBlock ***c_codeblocks) {
	int *subbandImg = new int[subband->w * subband->h];
	if (!subband->isNode) {
		leaf_(subband, c_codeblocks, subbandImg);
	}
	else {
		int **childrenParts = new int*[4];
		childrenParts[0] = reconstructLvlRecursive(subband->LL, c_codeblocks);
		childrenParts[1] = reconstructLvlRecursive(subband->HL, c_codeblocks);
		childrenParts[2] = reconstructLvlRecursive(subband->LH, c_codeblocks);
		childrenParts[3] = reconstructLvlRecursive(subband->HH, c_codeblocks);
		wavelet2DReconstruction_(subband, childrenParts, subbandImg);
	}
	return subbandImg;
}

void interleave(Subband *subband, int **childrenParts, int *subbandImg) {
	for (int i = 0; i < subband->h / 2; i++) {
		for (int j = 0; j < subband->w / 2; j++) {
			int i_ = 2 * i, j_ = 2 * j;
			subbandImg[i_ * subband->w + j_] = childrenParts[0][i * subband->w / 2 + j];
			i_ = 2 * i, j_ = 2 * j + 1;
			subbandImg[i_ * subband->w + j_] = childrenParts[1][i * subband->w / 2 + j];
			i_ = 2 * i + 1, j_ = 2 * j;
			subbandImg[i_ * subband->w + j_] = childrenParts[2][i * subband->w / 2 + j];
			i_ = 2 * i + 1, j_ = 2 * j + 1;
			subbandImg[i_ * subband->w + j_] = childrenParts[3][i * subband->w / 2 + j];
		}
	}
}

int *oneD_SR(int *y, int len) {
	int *x = new int[len];

	// First we take care of the symmetric extention to the left of the signal without the y_ext lookup table specified in itut. 
	// As there is no coefficient to the left of y[0], we mirror y[1] and treat it as y[-1] (i.e. y[-1] = y[1]). So the pattern becomes:
	// x[0] = y[0] - ((y[-1] + y[1] + 2) >> 2) = y[0] - ((y[1] + y[1] + 2) >> 2) = y[0] - ((2y[1] + 2) >> 2)
	// We can simplify the nominator and the denominator of the expression in parenthesis by the common factor of 2 
	// (shifting a number two bit positions to the right is like dividing it by 4, shifting is just MUCH quicker), 
	// and thus the pattern becomes x[0] = y[0] - ((y[1] + 1) >> 1) 

	x[0] = y[0] - ((y[1] + 1) >> 1); 
	for (int i = 2; i < len - 1; i += 2) {
		x[i] = y[i] - ((y[i-1] + y[i+1] + 2) >> 2); //even samples calculation (1)
	}
	for(int i = 1; i < len - 1; i+=2){
		x[i] = y[i] + ((x[i-1] + x[i+1]) >> 1); //odd samples calculation (2)
	}

	// Symmetric extention to the right of the x. The discussion above also applies here.Only this time we consider the last coefficient of x signal 
	// (i.e. x[len-1]), and as there is no x[len] coeffieient, we mirror x[len-2] to act like it.
	// So we have x[len] = x[len-2]. Just apply this extention to the pattern (2) and you get the formula for calculating x[len-1].

	x[len-1] = y[len-1] + x[len-2]; 
	return x;
}

void horSR(Subband *subband, int *subbandImg) {
	int *y = new int[subband->w];
	for (int i = 0; i < subband->h; i++) {
		for (int j = 0; j < subband->w; j++) {
			y[j] = subbandImg[i * subband->w + j];
		}
		int *x = oneD_SR(y, subband->w);
		for (int j = 0; j < subband->w; j++) {
			subbandImg[i * subband->w + j] = x[j];
		}
	}
}


void verSR(Subband *subband, int *subbandImg) {
	int *y = new int[subband->h];
	for (int j = 0; j < subband->w; j++) {
		for (int i = 0; i < subband->h; i++) {
			y[i] = subbandImg[i * subband->w + j];
		}
		int *x = oneD_SR(y, subband->h);
		for (int i = 0; i < subband->h; i++) {
			subbandImg[i * subband->w + j] = x[i];
		}
	}
}

void InverseWaveletTransform::wavelet2DReconstruction_(Subband *subband, int **childrenParts, int *subbandImg) {
	interleave(subband, childrenParts, subbandImg);
	horSR(subband, subbandImg);
	verSR(subband, subbandImg);
}


//============================================================================================================================

//if the experimental approach below is to be used, please add these two lines to the for loop in the inverseSubbandCodeblocks
//function (i.e. instead the line: reconstructedComponents[c] = reconstructLvlRecursive(componentRoots[c], cblks[c]); )
//reconstructedComponents[c] = new int[mr->Xsiz * mr->Ysiz];
//waveletTreeReconstruction(reconstructedComponents[c], componentRoots[c], cblks[c]);


void synthetize_lpf(int *lowSig, int lowOff, int lowLen, int lowStep,
	int* highSig, int highOff, int highLen, int highStep,
	int* outSig, int outOff, int outStep) {
	int i;
	int outLen = lowLen + highLen; //Length of the output signal
	int iStep = 2 * outStep; //Upsampling in outSig
	int ik; //Indexing outSig
	int lk; //Indexing lowSig
	int hk; //Indexing highSig  
	/* Generate even samples (inverse low-pass filter) */
	lk = lowOff;
	hk = highOff;
	ik = outOff;
	//Handle tail boundary effect. Use symmetric extension.
	if (outLen>1) {
		outSig[ik] = lowSig[lk] - ((highSig[hk] + 1) >> 1);
	}
	else {
		outSig[ik] = lowSig[lk];
	}
	lk += lowStep;
	hk += highStep;
	ik += iStep;
	//Apply lifting step to each "inner" sample.
	for (i = 2; i < outLen - 1; i += 2) {
		outSig[ik] = lowSig[lk] -
			((highSig[hk - highStep] + highSig[hk] + 2) >> 2);

		lk += lowStep;
		hk += highStep;
		ik += iStep;
	}
	//Handle head boundary effect if input signal has odd length.
	if ((outLen % 2 == 1) && (outLen>2)) {
		outSig[ik] = lowSig[lk] - ((2 * highSig[hk - highStep] + 2) >> 2);
	}
	/* Generate odd samples (inverse high pass-filter) */
	hk = highOff;
	ik = outOff + outStep;
	//Apply first lifting step to each "inner" sample.
	for (i = 1; i < outLen - 1; i += 2) {
		// Since signs are inversed (add instead of substract)
		// the +1 rounding dissapears.
		outSig[ik] = highSig[hk] +
			((outSig[ik - outStep] + outSig[ik + outStep]) >> 1);

		hk += highStep;
		ik += iStep;
	}
	//Handle head boundary effect if input signal has even length.
	if (outLen % 2 == 0 && outLen>1) {
		outSig[ik] = highSig[hk] + outSig[ik - outStep];
	}
}

void InverseWaveletTransform::wavelet2DReconstruction(Subband *suband, int *img) {
	int *buf;
	int ulx, uly, w, h;
	ulx = suband->oX;
	uly = suband->oY;
	w = suband->w;
	h = suband->h;
	buf = new int[(w >= h) ? w : h];
	//Perform the horizontal reconstruction
	for (int i = 0; i<h; i++) {
		for (int j = 0; j < w; j++) {
			buf[j] = img[i * mr->Xsiz + j];
		}
		synthetize_lpf(buf, 0, (w + 1) / 2, 1, buf, (w + 1) / 2, w / 2, 1,	img, i * mr->Xsiz, 1);
	}
	//Perform the vertical reconstruction
	for (int j = 0; j<w; j++) {
		for (int i = h - 1, k = i * mr->Xsiz + j; i >= 0; i--, k -= mr->Xsiz) {
			buf[i] = img[k];
		}
		synthetize_lpf(buf, 0, (h + 1) / 2, 1, buf, (h + 1) / 2, h / 2, 1, img, j, mr->Xsiz);
	}
	
}

void InverseWaveletTransform::leaf(Subband *subband, CodeBlock ***c_codeblocks, int *img) {
	CodeBlock *cblk = c_codeblocks[subband->rLvl][subband->type];
	for (int i = cblk->h - 1; i >= 0; i--) {
		int srcPos = i * cblk->w;
		int destPos = (cblk->oY + i) * mr->Xsiz + cblk->oX;
		for (int j = 0; j < cblk->w; j++) {
			img[destPos + j] = cblk->coefficients[srcPos + j];
		}
	}
}

void InverseWaveletTransform::waveletTreeReconstruction(int* img, Subband *sb, CodeBlock ***c_codeblocks) {
	CodeBlock *subbData = c_codeblocks[sb->rLvl][sb->type];
	if (!sb->isNode) {
		leaf(sb, c_codeblocks, img);
	}
	else {
		waveletTreeReconstruction(img, sb->LL, c_codeblocks);
		waveletTreeReconstruction(img, sb->HL, c_codeblocks);
		waveletTreeReconstruction(img, sb->LH, c_codeblocks);
		waveletTreeReconstruction(img, sb->HH, c_codeblocks);
		wavelet2DReconstruction(sb, img);
	}
}