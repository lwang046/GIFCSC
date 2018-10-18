#include "GifCSC/Cathode/interface/cathodePrintOuts.h"
#include "DataFormats/CSCDigi/interface/CSCCLCTDigi.h"

#include <stdio.h>

void CLCT_print(const CSCCLCTDigi& clctdigi){
	printf ("\t\tCLCT: v:%2d   q:%2d   p:%2d   inStr:%3d   cfeb:%2d   kStr:%3d   bx:%4d   fbx:%5d\n",
			clctdigi.isValid(), // v
			clctdigi.getQuality(),  // q
			clctdigi.getPattern(),  // p
			clctdigi.getStrip(),    // inStr [0..31] - halfstrip
			clctdigi.getCFEB(),     // cfeb
			clctdigi.getKeyStrip(), // kStr          - halfstrip
			clctdigi.getBX(),
			clctdigi.getFullBX());//
}
