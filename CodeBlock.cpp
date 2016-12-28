#include "common.h"

CodeBlock::CodeBlock() {

}

void CodeBlock::initDataSet() {
	for (int i = 0; i < cbLen; i++) {
		out_data[i] = 0;
	}
}