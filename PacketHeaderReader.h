#pragma once
#include "common.h"

class PacketHeaderReader {
	public:
		PacketHeaderReader(unsigned char* data);
		int readBit();
		int readBits(int n);
		void synch();
		void readPacketBody(int bodyLen, unsigned char *buf);

		int dataPos = 0;

	private:
		unsigned char* data;
		/** The current bit buffer */
		int bbuf = 0;

		/** The position of the next bit to read in the bit buffer (0 means
		*  empty, 8 full) */
		int bpos = 0;

		/** The next bit buffer, if bit stuffing occurred (i.e. current bit
		*  buffer holds 0xFF) */
		int nextbbuf = 0;
};