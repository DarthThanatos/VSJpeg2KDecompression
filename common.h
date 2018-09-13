#pragma once

#define _CRT_SECURE_NO_WARNINGS
#include <iostream> 
#include <climits> 
#include <Windows.h>
#include <conio.h>
#include <map>
#include <cstdlib>
#include <string>
#include <vector>
#include <array>
#include <list>
using namespace std;
#include "CodeBlock.h"
#include <math.h>
#include "StreamReader.h"
#include "Subband.h"
#include "MetadataReader.h"
#include "MQDecoder.h"
#include "PacketHeaderReader.h"
#include "TagTreeDecoder.h"
#include "PacketDecoder.h"
#include "EntropyDecoder.h"
#include "InverseWaveletTransform.h"

const int ALL_C = 3, ALL_R = 6, ALL_S = 4;
enum FILTERS { F5x3, F9x3 };

typedef struct {
	int sign;
	unsigned short precision;
	unsigned short XRsiz, YRsiz;
} SXY;

/* MACROS */
#define CHECK(x) \
  do { \
		fprintf(stdout, "checking if %s\n", #x); \
        if (!(x)) { \
            fprintf(stderr,"%s %s:%d: ",__FILE__, __func__, __LINE__); \
            perror(#x); \
            exit(-1); \
        } \
    } while(0) 


/* CLASSES */
template <typename TYPE_FIRST, typename TYPE_SECOND>
inline pair<TYPE_FIRST, TYPE_SECOND> Pair(TYPE_FIRST key, TYPE_SECOND value) {
	return pair <TYPE_FIRST, TYPE_SECOND>(key, value);
}

template <typename TYPE_FIRST, typename TYPE_SECOND>
class Map : public map<TYPE_FIRST, TYPE_SECOND> {
public:
	void put(TYPE_FIRST key, TYPE_SECOND value) {
		map<TYPE_FIRST, TYPE_SECOND>::insert(Pair(key, value));
	}
};
