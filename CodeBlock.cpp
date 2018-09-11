#include "common.h"

string CodeBlock::toString() {
	string str = "(ulx,uly,w,h)= (" + to_string(oX) + "," + to_string(oY) + "," + to_string(w) + "," + to_string(h);
	str += ") " + to_string(msbSkipped) + " MSB bit(s) skipped\n";
	str += "\tl: 0, len:" + to_string(cbLen) + ", ntp:" + to_string(ntp) + "\n";
	return str;
}

void CodeBlock::printCoefficients() {
	cout << "(ulx,uly,w,h)= (" + to_string(oX) + "," + to_string(oY) + "," + to_string(w) + "," + to_string(h) << ")" << endl;
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			cout << coefficients[i * w + j] << (j != w - 1 ? " " : "");
		}
		cout << "\n";
	}
}