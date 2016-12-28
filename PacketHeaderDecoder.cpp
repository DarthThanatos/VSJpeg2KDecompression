#include "common.h"

bool PacketHeaderDecoder::readPktHead(int c, int r, Subband *subband)  {

	int passtype;               // coding pass type
	int tmp, tmp2, totnewtp = 0, lblockCur, tpidx;
	int sumtotnewtp = 0;

	
	// If packet is empty there is no info in it (i.e. no code-blocks)
	int firstBit = in->readBit();
	if (firstBit == 0) {
		// No code-block is included, but usually there is one
		return false;
	}

	for (list<CodeBlock*>::iterator curCB = subband->cblocks->begin(); curCB != subband->cblocks->end(); curCB++) {

		TagTreeDecoder tdIncl(in);
		TagTreeDecoder tdBD(in);
		// Read inclusion using tag-tree
		tdIncl.update(*curCB);
		// Read bitdepth using tag-tree
		tmp = tdBD.update(*curCB);
		(*curCB)->msbSkipped = tmp;
		(*curCB)->truncationPointsAmount = 1;
		// If not inclused
		if (in->readBit() != 1) {
			continue;
		}
		// Read new truncation points
		if (in->readBit() == 1) {// if bit is 1
			totnewtp++;
			// if next bit is 0 do nothing
			if (in->readBit() == 1) {//if is 1
				totnewtp++;

				tmp = in->readBits(2);
				totnewtp += tmp;

				// If next 2 bits are not 11 do nothing
				if (tmp == 0x3) { //if 11
					tmp = in->readBits(5);
					totnewtp += tmp;

					// If next 5 bits are not 11111 do nothing
					if (tmp == 0x1F) {//if 11111
						totnewtp +=
							in->readBits(7);
					}
				}
			}
		}
				
		// Reads lblock increment (common to all segments)
		while (in->readBit() != 0) {
			(*curCB)->lblock++;
		}
		(*curCB)->cbLen = in->readBits((*curCB)->lblock + log2(totnewtp));		
	}
	return false;
}

