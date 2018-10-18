#ifndef SegmentHistos_h
#define SegmentHistos_h

#include "GifCSC/basics/interface/basicHistos.h"
#include "DataFormats/CSCRecHit/interface/CSCSegmentCollection.h"
#include "DataFormats/CSCRecHit/interface/CSCRecHit2D.h"


class basicSegmentHistos:public basicHistos{
public:
        basicSegmentHistos(TDirectory * dout, const int debug=0);
        bool fillSegmentHistos(const CSCSegment& segments);
        bool fillSegmentHistos(short int NwiredigiEvt);
	bool fillSegmentHistos(const CSCRecHit2D& seg2Dhit);
private:
        bool book(); // the only what has to be rewritten from basicHistos
};
#endif

