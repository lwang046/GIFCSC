#ifndef cathodeHistos_h
#define cathodeHistos_h

#include "GifCSC/basics/interface/basicHistos.h"
#include "DataFormats/CSCDigi/interface/CSCCLCTDigi.h"

class basicCLCTHistos:public basicHistos{
public:
	basicCLCTHistos(TDirectory * dout, const int debug=0);
	bool fillCLCTHistos(const CSCCLCTDigi& clctdigi);
	bool fillCLCTHistos(short int NclctEvt, short int dBX);
private:
	bool book(); // the only what has to be rewritten from basicHistos
};
#endif
