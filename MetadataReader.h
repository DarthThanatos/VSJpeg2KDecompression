#pragma once
#include "common.h"

class MetadataReader {
public:
	MetadataReader(StreamReader *);
	void parseMetadata();
	void handleSIZ();
	void handleCOD();
	void handleQCD();
	void handleComment();
	void handleSOT();
	void handleQCC();
	void readCompressedImage();

	/*public fields*/

	char scod_byte;
	bool entropy_coder;
	bool sop_marker_used;
	bool eph_marker_used;
	char progression_order, multiple_component_transf_usage;
	char  code_block_style;
	char codeblock_exponent_width_offset_val, codeblock_exponent_height_offset_val;
	char transformation;
	unsigned short number_of_layers;
	int bit_stream_length;
	int number_of_decomposition_lvls;


	char sqcd_byte, guard_bits, quantization_type;

private:
	StreamReader *streamReader;
};