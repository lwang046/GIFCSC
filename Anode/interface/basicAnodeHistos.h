#ifndef AnodeHistos_h
#define AnodeHistos_h

#include "GifCSC/basics/interface/basicHistos.h"
#include "DataFormats/CSCDigi/interface/CSCALCTDigiCollection.h"
#include "DataFormats/CSCDigi/interface/CSCWireDigiCollection.h"
#include "DataFormats/CSCDigi/interface/CSCALCTDigi.h"
#include "DataFormats/CSCDigi/interface/CSCWireDigi.h"

class basicAnodeHistos:public basicHistos{
public:
	basicAnodeHistos(TDirectory * dout, const int debug=0);
	bool fillAnodeHistos(const CSCWireDigi& wiredigi);
	bool fillAnodeHistos(short int NwiredigiEvt);
private:
	bool book(); // the only what has to be rewritten from basicHistos
};
#endif
