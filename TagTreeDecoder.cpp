#include "common.h"

TagTreeDecoder::TagTreeDecoder(PacketHeaderReader * in, int w, int h) {
	this->in = in;
	// Initialize dimensions
	this->w = w;
	this->h = h;
	// Calculate the number of levels
	if (w == 0 || h == 0) {
		lvls = 0; // Empty tree
	}
	else {
		lvls = 1;
		while (h != 1 || w != 1) { // Loop until we reach root
			w = (w + 1) >> 1;
			h = (h + 1) >> 1;
			lvls++;
		}
	}
	// Allocate tree values and states
	treeV = new int*[lvls];
	treeS = new int*[lvls];
	w = this->w;
	h = this->h;
	for (int i = 0; i<lvls; i++) {
		treeV[i] = new int[h*w];
		treeS[i] = new int[h*w];
		for (int j = 0; j < h*w; j++) {
			treeV[i][j] = INT_MAX;
			treeS[i][j] = 0;
		}
		w = (w + 1) >> 1;
		h = (h + 1) >> 1;
	}
}

int TagTreeDecoder::update() {
	int k, tmin;
	int idx, ts, tv;
	int m, n;
	m = 0;
	n = 0;
	// Initialize
	k = lvls - 1;
	tmin = treeS[k][0];

	// Loop on levels
	idx = (m >> k)*((w + (1 << k) - 1) >> k) + (n >> k);
	while (true) {
		// Cache state and value
		ts = treeS[k][idx];
		tv = treeV[k][idx];
		if (ts < tmin) {
			ts = tmin;
		}
		while (true) {
			if (tv > ts) { // We are not done yet
				if (in->readBit() == 0) { // '0' bit => We know that 'value' > treeS[k][idx]
					ts++;
				}
				else { // '1' bit => We know that 'value' = treeS[k][idx]
					tv = ts;
				}
			}
			else { // We are done, we can set ts and get out
				break; // get out of this while
			}
		}
		// Update state and value
		treeS[k][idx] = ts;
		treeV[k][idx] = tv;
		// Update tmin or terminate
		if (k>0) {
			tmin = ts < tv ? ts : tv;
			k--;
			// Index of element for next iteration
			idx = (m >> k)*((w + (1 << k) - 1) >> k) + (n >> k);
		}
		else {
			// Return the updated value
			return tv;
		}
	}
}
