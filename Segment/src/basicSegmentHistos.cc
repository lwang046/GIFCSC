#include <utility>
#include <iostream>

#include "TDirectory.h"
#include "TH1F.h"

#include "GifCSC/Segment/interface/basicSegmentHistos.h"
#include "GifCSC/basics/interface/basicHistosNaming.h"

basicSegmentHistos::basicSegmentHistos(TDirectory * dout, const int debug):basicHistos(dout, debug){
        if(debuglevel>1){
                std::cout << " basicSegmentHistos::basicSegmentHistos   : directory pointer " << &outdir << "; directory name: ";
                std::cout << outdir->GetPath() << std::endl;
        };
        book();
        if(debuglevel)std::cout << " basicSegmentHistos::basicSegmentHistos    : => completed " << std::endl;
}

bool basicSegmentHistos::book(){
        outdir->cd(); // important
        TH1F * h;
        TString htitle, hdescr, hfunc;
	//0
        htitle="NperEvt"; hfunc="evaluated"; hdescr="number of segments per event";
        h = new TH1F(htitle.Data(), htitle.Data(), 21, -1, 20);
        histosWithDescritions.push_back(std::make_pair(h, new basicHistosNaming(htitle.Data(), htitle.Data(), hfunc.Data(), hdescr.Data())));
	//1
	htitle="SegmentWG"; hfunc="CSCSegmentCollection.hitWire()"; hdescr="Segment hitWire";
	h = new TH1F(htitle.Data(), htitle.Data(), 49, -1, 48);
	histosWithDescritions.push_back(std::make_pair(h, new basicHistosNaming(htitle.Data(), htitle.Data(), hfunc.Data(), hdescr.Data())));
	//2
        htitle="SegmentPositionX"; hfunc="CSCSegmentCollection.localPosition().x()"; hdescr="Segment Position X";
        h = new TH1F(htitle.Data(), htitle.Data(), 101, -1, 100);
        histosWithDescritions.push_back(std::make_pair(h, new basicHistosNaming(htitle.Data(), htitle.Data(), hfunc.Data(), hdescr.Data())));
	//3
        htitle="SegmentPositionY"; hfunc="CSCSegmentCollection.localPosition().y()"; hdescr="Segment Position Y";
        h = new TH1F(htitle.Data(), htitle.Data(), 101, -1, 100);
        histosWithDescritions.push_back(std::make_pair(h, new basicHistosNaming(htitle.Data(), htitle.Data(), hfunc.Data(), hdescr.Data())));
	//4
        htitle="SegmentChi2"; hfunc="CSCSegmentCollection.chi2()"; hdescr="Segment Chi2";
        h = new TH1F(htitle.Data(), htitle.Data(), 101, -1, 100);
        histosWithDescritions.push_back(std::make_pair(h, new basicHistosNaming(htitle.Data(), htitle.Data(), hfunc.Data(), hdescr.Data())));
	//5
        htitle="SegmentDegreesOfFreedom"; hfunc="CSCSegmentCollection.degreesOfFreedom()"; hdescr="Segment Degrees of freedom";
        h = new TH1F(htitle.Data(), htitle.Data(), 51, -1, 50);
        histosWithDescritions.push_back(std::make_pair(h, new basicHistosNaming(htitle.Data(), htitle.Data(), hfunc.Data(), hdescr.Data())));


        if(debuglevel){
                std::cout << " basicSegmentHistos::book         : booking ends with following content in the directory :" << std::endl;
                outdir->Print(); // does not work in CMSSW??!
                for (std::vector<std::pair<TH1F*, basicHistosNaming*>>::iterator it=histosWithDescritions.begin(); it!=histosWithDescritions.end(); ++it)
                        (*it).second->Print();
        };
        return true;
};

bool basicSegmentHistos::fillSegmentHistos(const CSCSegment& segments){

        if(histosWithDescritions.size()<6) return false;

	//std::cout << "***********debug: " << segments.chi2() << std::endl;

        histosWithDescritions.at(2).first->Fill(segments.localPosition().x());
        histosWithDescritions.at(3).first->Fill(segments.localPosition().y());
        histosWithDescritions.at(4).first->Fill(segments.chi2());
        histosWithDescritions.at(5).first->Fill(segments.degreesOfFreedom());

        return true;
};

bool basicSegmentHistos::fillSegmentHistos(short int NclctEvt=-1){
        if(histosWithDescritions.size()<1) return false;

        histosWithDescritions.at(0).first->Fill(NclctEvt);
        return true;
}

bool basicSegmentHistos::fillSegmentHistos(const CSCRecHit2D& seg2Dhit){

        if(histosWithDescritions.size()<2) return false;

        //std::cout << "***********debug2: " << seg2Dhit.hitWire() << std::endl;

	histosWithDescritions.at(1).first->Fill(seg2Dhit.hitWire());

        return true;
};
      

