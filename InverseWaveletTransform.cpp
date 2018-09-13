#include "common.h"


InverseWaveletTransform::InverseWaveletTransform(CodeBlock ****cblks, MetadataReader *mr) {
	this->componentRoots = mr->componentsSubbandRoots;
	this->cblks = cblks;
	this->mr = mr;
}

int **InverseWaveletTransform::inverseSubbandCodeblocks() {
	int **reconstructedComponents = new int*[ALL_C];
	for (int c = 0; c < ALL_C; c++) {
		//reconstructedComponents[c] = reconstructLvlRecursive(componentRoots[c], cblks[c]);

		reconstructedComponents[c] = new int[mr->Xsiz * mr->Ysiz];
		waveletTreeReconstruction(reconstructedComponents[c], componentRoots[c], cblks[c]);
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

void printInterleave(Subband *subband, int **childrenParts, int *subbandImg) {
	for (int ch = 0; ch < 4; ch++) {
		cout << endl << "child: " << ch << endl;
		for (int i = 0; i < subband->h / 2; i++) {
			for (int j = 0; j < subband->w / 2; j++) {
				cout << childrenParts[ch][i * subband->w / 2 + j] << " ";
			}
			cout << endl;
		}
	}
	cout << endl << "interleaved: " << endl;
	for (int i = 0; i < subband->h; i++) {
		for (int j = 0; j < subband->w; j++) {
			cout << subbandImg[i * subband->w + j] << " ";
		}
		cout << endl;
	}

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
	if (subband->rLvl == 1) {
		printInterleave(subband, childrenParts, subbandImg);
	}
}

void printExt(int *y, int *y_ext, int *x, int y_len, string debug) {
	cout << endl << debug << " y: ";
	for (int i = 0; i < y_len; i++) {
		cout << y[i] << " ";
	}
	cout << endl<< debug  << " y_ext: ";
	for (int i = 0; i < y_len+3; i++) {
		cout << y_ext[i] << " ";
	}
	cout << endl << debug << " x: ";
	for (int i = 0; i < y_len; i++) {
		cout << x[i] << " ";
	}
	cout << endl;
}

int *oneD_SR(int *y, int len) {
	int *y_ext = new int[len + 3];  
	for (int i = 1; i < len + 1; i++) {
		y_ext[i] = y[i-1];
	}
	y_ext[0] = y[1];
	y_ext[len + 1] = y[len - 2];
	y_ext[len+ 2] = y[len - 3];
	int *x = new int[len];

	for (int i = 1; i < (len +1)/ 2 + 1; i++) {
		x[2 * (i-1)] = y_ext[2 * i] - floor((y_ext[2 * i - 1] + y_ext[2*i+1] + 2)/4.0);
	}
	for (int i = 1; i < (len + 1) / 2 + 1; i++) {
		x[2 * (i - 1) + 1] = y_ext[2 * i + 1] + floor((x[2 * (i - 1)] + (2 * (i - 1) + 2 < len ? x[2 * (i - 1) + 2] : 0) ) / 2.0);
	}
	return x;
}


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

	//Initialize counters
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

	//Initialize counters
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
	// Copy the data line by line
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
		//Perform the 2D wavelet decomposition of the current subband
		wavelet2DReconstruction(sb, img);
	}
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
