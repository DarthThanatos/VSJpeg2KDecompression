#include "common.h"

PacketHeaderReader::PacketHeaderReader(unsigned char* data) {
	this->data = data;
}
void PacketHeaderReader::synch() {
	bbuf = 0;
	bpos = 0;
}

void PacketHeaderReader::readPacketBody(int bodyLen, unsigned char *buf) {
	for (int i = 0; i < bodyLen; i++) {
		buf[i] = data[dataPos++];
	}
}

int PacketHeaderReader::readBit() {
	if (bpos == 0) { // Is bit buffer empty?
		if (bbuf != 0xFF) { // No bit stuffing
			bbuf = data[dataPos++];
			bpos = 8;
			if (bbuf == 0xFF) { // If new bit stuffing get next byte
				nextbbuf = data[dataPos++];
			}
		}
		else { // We had bit stuffing, nextbuf can not be 0xFF
			bbuf = nextbbuf;
			bpos = 7;
		}
	}
	return (bbuf >> --bpos) & 0x01;

}

int PacketHeaderReader::readBits(int n) {

	int bits; 
	if (n <= bpos) {
		return (bbuf >> (bpos -= n)) & ((1 << n) - 1);
	}
	else {
		bits = 0;
		do {
			// Get all the bits we can from the bit buffer
			bits <<= bpos;
			n -= bpos;
			bits |= readBits(bpos);
			// Get an extra bit to load next byte (here bpos is 0)
			if (bbuf != 0xFF) { // No bit stuffing
				bbuf = data[dataPos++];
				bpos = 8;
				if (bbuf == 0xFF) { // If new bit stuffing get next byte
					nextbbuf = data[dataPos++];
				}
			}
			else { // We had bit stuffing, nextbuf can not be 0xFF
				bbuf = nextbbuf;
				bpos = 7;
			}
		} while (n > bpos);
		// Get the last bits, if any
		bits <<= n;
		bits |= (bbuf >> (bpos -= n)) & ((1 << n) - 1);
		// Return result
		return bits;
	}
}