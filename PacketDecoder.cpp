#include "common.h"

struct DecoderTables {
	int ***lblock;
	TagTreeDecoder ****ttIncl;
	TagTreeDecoder ****ttMaxBP;
	CodeBlock ****cblkI;
};

DecoderTables *initDecoderTables(PacketHeaderReader *phr) {
	DecoderTables *dts = new DecoderTables();
	dts->lblock = new int **[ALL_C];
	dts->ttIncl = new TagTreeDecoder ***[ALL_C];
	dts->ttMaxBP = new TagTreeDecoder ***[ALL_C];
	dts->cblkI = new CodeBlock ***[ALL_C];
	for (int c = 0; c < ALL_C; c++) {
		dts->lblock[c] = new int*[ALL_R];
		dts->ttIncl[c] = new TagTreeDecoder**[ALL_R];
		dts->ttMaxBP[c] = new TagTreeDecoder**[ALL_R];
		dts->cblkI[c] = new CodeBlock**[ALL_R];
		for (int r = 0; r < ALL_R; r++) {
			dts->lblock[c][r] = new int[ALL_S];
			dts->ttIncl[c][r] = new TagTreeDecoder*[ALL_S];
			dts->ttMaxBP[c][r] = new TagTreeDecoder*[ALL_S];
			dts->cblkI[c][r] = new CodeBlock*[ALL_S];
			for (int s = 0; s < ALL_S; s++) {
				dts->lblock[c][r][s] = 3;
				dts->ttIncl[c][r][s] = new TagTreeDecoder(phr, 1, 1);
				dts->ttMaxBP[c][r][s] = new TagTreeDecoder(phr, 1, 1);
				dts->cblkI[c][r][s] = NULL;
			}
		}
	}
	return dts;
}

void printCBI(DecoderTables *dts) {
	for (int c = 0; c < ALL_C; c++) {
		for (int r = 0; r < ALL_R; r++) {
			int mins = (r == 0) ? 0 : 1;
			int maxs = (r == 0) ? 1 : 4;
			for (int s = mins; s < maxs; s++) {
				CodeBlock *cBlkInfo = dts->cblkI[c][r][s];
				cout << c << " " << r << " " << s << endl << cBlkInfo->toString();
			}
		}
	}
}

CodeBlock**** PacketDecoder::readData(MetadataReader *mr) {
	// assuming layer-resolutionLvl-component-position ordering of data in packets, with number of layers = 1 
	// as only a small image with 128x128 resolution as an input is assumed
	PacketHeaderReader *phr = new PacketHeaderReader(mr->sod);
	DecoderTables *dts = initDecoderTables(phr); //dimens: c-r-s
	for (int r = 0; r < ALL_R; r++) {
		for (int c = 0; c < ALL_C; c++) {
			int mins = (r == 0) ? 0 : 1;
			int maxs = (r == 0) ? 1 : 4;
			phr->synch();
			if (phr->readBit() != 0) { //non zero packet
				for (int s = mins; s < maxs; s++) {
					TagTreeDecoder *ttIncl = dts->ttIncl[c][r][s];
					TagTreeDecoder *tdBD = dts-> ttMaxBP[c][r][s];
					CodeBlock *cbi = dts->cblkI[c][r][s];

					if (cbi == NULL) {
						Subband *sb = mr->componentsSubbandRoots[c]->getSubbandAt(r, s);
						cbi = dts->cblkI[c][r][s] = new CodeBlock();
						cbi->w = sb->w; cbi->h = sb->h; cbi->oX = sb->oX; cbi->oY = sb->oY;
					}
					int included = ttIncl->update();
					if (included > 0) {
						cout << "error, more layers than 1 detected, something bad is going to happen soon..." << endl;
						continue;
					}
					cbi->msbSkipped = tdBD->update();
					int npasses = 1;
					if (phr->readBit() == 1) {// if bit is 1
						npasses++;

						// if next bit is 0 do nothing
						if (phr->readBit() == 1) {//if is 1
							npasses++;

							int tmp = phr->readBits(2);
							npasses += tmp;

							// If next 2 bits are not 11 do nothing
							if (tmp == 0x3) { //if 11
								tmp = phr->readBits(5);
								npasses += tmp;
								// If next 5 bits are not 11111 do nothing
								if (tmp == 0x1F) { //if 11111
									npasses += phr->readBits(7);
								}
							}
						}
					}
					cbi->ntp = npasses;
					// Reads lblock increment (common to all segments)
					while (phr->readBit() != 0) {
						dts->lblock[c][r][s]++;
					}
					cbi->cbLen = phr->readBits(dts->lblock[c][r][s] + floor(log2(npasses)));
				}
				//read packet body after all subbands in a packet header has been iterated over in the header reading phase
				for (int s = mins; s < maxs; s++) {
					CodeBlock *cbi = dts->cblkI[c][r][s];
					cbi->data = new unsigned char[cbi->cbLen];
					phr->readPacketBody(cbi->cbLen, cbi->data);
				}
			}
		}
	}
	//printCBI(dts);
	return dts->cblkI;
}
