#include "common.h"


Subband::Subband(int lvl, int totalDecompLvls, int type, int w, int h, int oX, int oY) {
	this->totalDecompLvls = totalDecompLvls;
	this->type = type;
	this->w = w;
	this->h = h;
	this->oX = oX;
	this->oY = oY;
	rLvl = lvl;
	if (lvl != 0 && type == WT_ORIENT_LL) {
		childrenCount = 4;
		allChildren = new Subband*[childrenCount];
		LL = allChildren[0] = new Subband(lvl - 1, totalDecompLvls, WT_ORIENT_LL, w >> 1, h >> 1, 0, 0);
		HL = allChildren[1] = new Subband(lvl, totalDecompLvls, WT_ORIENT_HL, w >> 1, h >> 1, w >> 1, 0);
		LH = allChildren[2] = new Subband(lvl, totalDecompLvls, WT_ORIENT_LH, w >> 1, h >> 1, 0, h >> 1);
		HH = allChildren[3] = new Subband(lvl, totalDecompLvls, WT_ORIENT_HH, w >> 1, h >> 1, w >> 1, h >> 1);
	}
	this->isNode = childrenCount != 0;
}

Subband* Subband::getSubbandAt(int r, int s) {
	//cout << "r: " << r << " rlvl: " << rLvl << " s: " << s << " id: " << id <<  " children:" << childrenCount << endl;
	if (rLvl == r && s == type) { return this; }
	if (rLvl == r  && s > 0) 
		return childrenCount - 1 < s  ? NULL : allChildren[s];
	return LL->getSubbandAt(r, s);
}

string typeToString(int type) {
	switch (type) {
		case WT_ORIENT_LL: return "ll";
		case WT_ORIENT_HL: return "hl";
		case WT_ORIENT_LH: return "lh";
		case WT_ORIENT_HH: return "hh";
	}
}


string Subband::toString() {
	string str = "";
	for (int i = 0; i < totalDecompLvls - rLvl ; i++) { str += "|"; }
	str += typeToString(type) + "(" + to_string(type) + ")";
	str += " rlvl = " + to_string(rLvl) 
		+ "; dLvl = " + to_string(totalDecompLvls - rLvl) 
		+ "; w = " + to_string(w)
		+ "; h = " + to_string(h)
		+ "; oX = " + to_string(oX)
		+ "; oY = " + to_string(oY) + "\n";
	if (rLvl != 0 && type == WT_ORIENT_LL) {
		str += LL->toString();
			str += HL->toString();
			str += LH->toString();
			str += HH->toString();
		
	}
	return str;
}

void Subband::printCoefficients(int *coefficients) {
	cout << "(ulx,uly,w,h)= (" << oX << "," << oY << "," << w << "," << h << ")" << endl;
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			cout << coefficients[i * w + j] << (j != w - 1 ? " " : "");
		}
		cout <<"\n";
	}
}
