#pragma once
#include "common.h"

class InverseWaveletTransform {
	public:
		InverseWaveletTransform(CodeBlock ****cblks, MetadataReader *mr);
		int **inverseSubbandCodeblocks();

	private:
		int *reconstructLvlRecursive(Subband *subband, CodeBlock ***c_codeblocks);
		void waveletTreeReconstruction(int* img, Subband *sb, CodeBlock ***c_codeblocks);
		void wavelet2DReconstruction(Subband *suband, int *img);
		void wavelet2DReconstruction_(Subband *subband, int **childrenParts, int *subbandImg);
		void leaf(Subband *subband, CodeBlock ***c_codeblocks, int *subbandImg);
		void leaf_(Subband *subband, CodeBlock ***c_codeblocks, int *subbandImg);

		Subband **componentRoots;
		CodeBlock ****cblks;
		MetadataReader *mr;
};