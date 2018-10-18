#include <iostream>
#include "GifCSC/Segment/interface/Segment.h"
//edm::EDGetTokenT<CSCSegmentCollection> 
Segment::Segment(short unsigned chamber, edm::EDGetTokenT<CSCSegmentCollection> segmenttoken, TDirectory *outputroot, short unsigned debuglevel)
					 : chamberType(chamber), commonCurrentEvtN(-1), nSegments(0), debug(debuglevel){
	//std::cout << "here1: " << alct_token.isUninitialized() << "\t" << alct_token.index() << std::endl;
	segment_token = segmenttoken;
        seg_histos = NULL; histogramming = false;
        if(outputroot!=NULL){
                if(debug) std::cout << "creating histogram subdirectory in directory " << outputroot->GetPath() << std::endl;
                TDirectory *outdir = outputroot->mkdir("hSeg");
                if(outdir->cd()){
                        seg_histos    = new basicSegmentHistos(outdir, debug);
                        histogramming = true;
                }else{std::cout << "Segment can not create directory for basic segment historgrams in file " << outputroot->GetPath() << std::endl;}
        }else{
                std::cout << "Segment will not save basic historgrams since no directory was prepared!\n";
        };
	//std::cout << "here2: " << alct_token.isUninitialized() << "\t" << alct_token.index() << std::endl;
}

bool Segment::end(){
        bool ok = true;
        if(histogramming){
                ok = (ok && seg_histos->write());
        };
        return ok;
}

Segment::~Segment(){
	delete seg_histos;
}

bool Segment::bookSegmentHistos(){
	bool ok = false;
	return ok;
}
bool Segment::writeSegmentHistos(){
	bool ok = false;
	return ok;
}
bool Segment::cleanSegmentHistos(){
	bool ok = false;
	return ok;
}

bool Segment::segmentInEvent(const edm::Event& iEvent){
	edm::Handle<CSCSegmentCollection> segments;
        iEvent.getByToken(segment_token, segments);
        if(debug) std::cout << "Segment size: " << segments->size() << std::endl;

	WG_Seg.clear(); nSegments = 0;

        for (CSCSegmentCollection::const_iterator dSiter=segments->begin(); dSiter!=segments->end(); dSiter++) {
                CSCDetId sid = (CSCDetId)(*dSiter).cscDetId();
                if(sid.chamber()!=chamberType){
			if(debug) std::cout << "chamber sid" << sid.chamber() << ": chamber type" << chamberType << std::endl; 
			continue;
		}

                nSegments++;

                if(debug) std::cout   <<  "Segment from : chamber " << sid.chamber()
                                                          << " station " << sid.station()
                                                          << " ring    " << sid.ring()
                                                          << " layer   " << sid.layer() << std::endl;
                if(debug)
                        std::cout << "Segment Position: X=" << (*dSiter).localPosition().x()  << "; Y=" << (*dSiter).localPosition().y() << std::endl;
		if(histogramming) seg_histos->fillSegmentHistos(*dSiter);

    			const std::vector<CSCRecHit2D> seg2DhitVec = (*dSiter).specificRecHits();
    			for(std::vector<CSCRecHit2D>::const_iterator seg2Dhit=seg2DhitVec.begin(); seg2Dhit!=seg2DhitVec.end(); ++seg2Dhit){
          			if(debug) std::cout << " WG      " << (*seg2Dhit).hitWire() << std::endl;
				WG_Seg.push_back((*seg2Dhit).hitWire());
				if(histogramming) seg_histos->fillSegmentHistos(*seg2Dhit);
    			}
                
        };
	if(histogramming) seg_histos->fillSegmentHistos(nSegments);
        if(debug) std::cout << "Segments in event: " <<  segments->size() << "/ Segments in chamber: " << nSegments << std::endl;
        return true;
}




