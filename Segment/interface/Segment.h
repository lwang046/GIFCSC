#ifndef csc_segments_h
#define csc_segments_h

//#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDConsumerBase.h"       // edm::EDGetTokenT
#include "FWCore/ParameterSet/interface/ParameterSet.h"   // edm::ParameterSet
#include "FWCore/Framework/interface/Event.h"                // edm::Event
#include "DataFormats/CSCRecHit/interface/CSCSegmentCollection.h"
#include "DataFormats/CSCRecHit/interface/CSCRecHit2D.h"
#include "DataFormats/MuonDetId/interface/CSCDetId.h"
#include "TH1F.h"
#include "TH2F.h"

#include "GifCSC/Segment/interface/basicSegmentHistos.h"

class Segment: public edm::EDConsumerBase{
public:
	Segment(short unsigned chamber, edm::EDGetTokenT<CSCSegmentCollection> segmenttoken, TDirectory *outputroot, short unsigned debuglevel);
	~Segment();
private:
	int  chamberType;                // 1 - ME1/1; 2 - ME2/1; >10 - P5 (not yet); signed because of CSCDetId.chamber is signed
	long 	commonCurrentEvtN;       // current event N, -1 if not initialized
	unsigned int nSegments; 	 // event counter segments
	short unsigned int debug;            // 0 - no debug
        std::vector<int> WG_Seg;
	edm::EDGetTokenT<CSCSegmentCollection> segment_token;

private:
        basicSegmentHistos * seg_histos;
        bool histogramming;
	TH2F  * h2dSeg;       // 2D histos of segments belonging to CSCSegmentCollection
	TH1F  * hNofSeg;      // number of segments per event
	TH1F  * hsegChi2;     // chi2
 	TH1F  * hsegNof2Dhits;

private:
	bool bookSegmentHistos();
	bool writeSegmentHistos();
	bool cleanSegmentHistos();

public:
	bool segmentInEvent(const edm::Event& iEvent);
	bool end();
        int GetNperEvt() {return nSegments; }
        std::vector<int> GetWG() {return WG_Seg; }
};
#endif
