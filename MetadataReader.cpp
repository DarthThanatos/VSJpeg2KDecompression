#include "common.h"

MetadataReader::MetadataReader(StreamReader* streamReader) {
	this->streamReader = streamReader;
}

void MetadataReader::handleSIZ() {
	printf("siz\n");
	short length = streamReader->getTwoByteLength();
	short capabilities = streamReader->readTwoBytes();
	Xsiz = streamReader->readFourBytes();
	Ysiz = streamReader->readFourBytes(); 
	unsigned int XOsiz = streamReader->readFourBytes(); 
	unsigned int YOsiz = streamReader->readFourBytes();
	unsigned int XTsiz = streamReader->readFourBytes();
	unsigned int YTsiz = streamReader->readFourBytes();
	unsigned int XTOsiz = streamReader->readFourBytes();
	unsigned int YTOsiz = streamReader->readFourBytes(); 
	unsigned short componentsAmount = streamReader->readTwoBytes();
	SXY *sxy = new SXY[componentsAmount];
	;
	for (unsigned short i = 0; i < componentsAmount; i++) {
		char get_byte = streamReader->readByte();
		sxy[i].XRsiz = sxy[i].YRsiz = 0;
		sxy[i].sign = get_byte >> 7;
		sxy[i].precision = (get_byte & 0x7) + 1;
		sxy[i].XRsiz = streamReader->readByte();
		sxy[i].YRsiz = streamReader->readByte();
		printf("sign = %d, precision = %d, xrsiz = %d, yrsiz = %d\n", sxy[i].sign, sxy[i].precision, sxy[i].XRsiz, sxy[i].YRsiz);
	}
	printf("xsiz = %u ysiz = %u\n", Xsiz, Ysiz);
	number_of_components = componentsAmount;
}

void MetadataReader::handleCOD() {
	printf("cod\n");
	unsigned short length = streamReader->getTwoByteLength();
	scod_byte = streamReader->readByte();
	entropy_coder = scod_byte & 0x1;
	sop_marker_used = scod_byte & 0x2;
	eph_marker_used = scod_byte & 0x4;
	progression_order = streamReader->readByte();
	number_of_layers = streamReader->readTwoBytes();
	multiple_component_transf_usage = streamReader->readByte();
	number_of_decomposition_lvls = streamReader->readByte();
	codeblock_exponent_width_offset_val = streamReader->readByte();
	codeblock_exponent_height_offset_val = streamReader->readByte();
	code_block_style = streamReader->readByte();
	transformation = streamReader->readByte();
	codeblock_height = 1 << (codeblock_exponent_height_offset_val + 2);
	codeblock_width = 1 << (codeblock_exponent_width_offset_val + 2);
	printf(
		"cb width = %d, cb height = %d, decomposition lvls = %d progression order = %d\n", 
		codeblock_width, 
		codeblock_height, 
		number_of_decomposition_lvls, 
		progression_order
	);
}

void MetadataReader::handleQCD() {
	printf("qcd\n");
	unsigned short length = streamReader->getTwoByteLength();
	sqcd_byte = streamReader->readByte(); 
	guard_bits = sqcd_byte >> 5;
	quantization_type = sqcd_byte & 0x5;
	int size = quantization_type == 0 ? 8 : 16;

}

void MetadataReader::handleComment() {
	printf("com\n");
	unsigned short length = streamReader->getTwoByteLength() ;
	unsigned short rCom = streamReader->readTwoBytes();
	unsigned char *characters = new unsigned char[length - 4];
	characters = streamReader->readNBytes(characters, length - 4);
	characters[length - 4] = '\0';
	printf("%s\n", characters);

}

void MetadataReader::handleSOT() {
	printf("sot\n");
	unsigned short length = streamReader->getTwoByteLength(); //it is always 10, but need to read it anyway
	unsigned short tile_index_Isot;
	int tile_length;
	char tile_index_TPsot, number_of_tile_in_cs;
	tile_index_Isot = streamReader->readTwoBytes();
	tile_length = streamReader->readFourBytes();
	tile_index_TPsot = streamReader->readByte();
	number_of_tile_in_cs = streamReader->readByte();
	bit_stream_length = tile_length - 14; //without fields above, and sot and sot markers
	printf("tile length = %d lsot = %d isot = %d tpsot = %d tile_number = %d\n", bit_stream_length, length, tile_index_Isot, tile_index_TPsot, number_of_tile_in_cs);
}

void MetadataReader::handleQCC() {
	printf("qcc\n");
}

void MetadataReader::readCompressedImage(){
	printf("sod\n");
	cout << "pos: " << ftell(streamReader->input) << endl;
	sod = new unsigned char[bit_stream_length];
	sod = streamReader->readNBytes(sod, bit_stream_length);
	printf("end of sod\n");
}

void MetadataReader::parseMetadata() {
	bool shouldContinue = true;
	unsigned char byte = 0, prev_byte = 0;
	while (shouldContinue) {
		byte = streamReader->readByte(); 
		if (prev_byte == 0xff) {
			switch (byte) {
			case 0xd9:
				break;
			case 0x4f:
				printf("soc\n");
				break;
			case 0x51:
				handleSIZ();
				break;
			case 0x52:
				handleCOD();
				break;
			case 0x64:
				handleComment();
				break;
			case 0x5c:
				handleQCD();
				break;
			case 0x5d:
				handleQCC();
				break;
			case 0x90:
				handleSOT();
				break;
			case 0x93:
				readCompressedImage(); //sod
				shouldContinue = false;
				break;
			default: break;
		}

		}
		prev_byte = byte;
	}
}

void MetadataReader::initSubbands() {
	componentsSubbandRoots = new Subband*[number_of_components];
	for (int i = 0; i < number_of_components; i++) {
		componentsSubbandRoots[i] = new Subband(
			number_of_decomposition_lvls,
			number_of_decomposition_lvls,
			WT_ORIENT_LL,
			Xsiz, Ysiz, 0, 0
		);
		cout << componentsSubbandRoots[i]->toString() << endl;
	}
}