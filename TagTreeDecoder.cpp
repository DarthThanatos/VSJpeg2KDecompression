#include "common.h"

TagTreeDecoder::TagTreeDecoder(StreamReader * in) {
	this->in = in;
}

int TagTreeDecoder::update(CodeBlock *cblk) {
	int k, tmin, t = 0;
	int idx, ts, tv;
	int m, n;
	m = cblk->m;
	n = cblk->n;
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
		while (t > ts) {
			if (tv >= ts) { // We are not done yet
				if (in->readBit() == 0) { // '0' bit
											// We know that 'value' > treeS[k][idx]
					ts++;
				}
				else { // '1' bit
						// We know that 'value' = treeS[k][idx]
					tv = ts++;
				}
				// Increment of treeS[k][idx] done above
			}
			else { // We are done, we can set ts and get out
				ts = t;
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
