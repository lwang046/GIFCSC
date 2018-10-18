#include <utility>
#include <iostream>

#include "TDirectory.h"
#include "TH1F.h"

#include "GifCSC/Anode/interface/basicAnodeHistos.h"
#include "GifCSC/basics/interface/basicHistosNaming.h"

//!!!TBD!!! does it work for two chambers???

basicAnodeHistos::basicAnodeHistos(TDirectory * dout, const int debug):basicHistos(dout, debug){
	if(debuglevel>1){
		std::cout << " basicAnodeHistos::basicAnodeHistos   : directory pointer " << &outdir << "; directory name: ";
		std::cout << outdir->GetPath() << std::endl;
	};
	book();
	if(debuglevel)std::cout << " basicAnodeHistos::basicAnodeHistos	   : => completed " << std::endl;
}



bool basicAnodeHistos::book(){
	outdir->cd(); // important
	TH1F * h;
	TString htitle, hdescr, hfunc;

	// 0
	htitle="NperEvt"; hfunc="evaluated"; hdescr="number of wiredigis per event";
	h = new TH1F(htitle.Data(), htitle.Data(), 11, -1, 10);
	histosWithDescritions.push_back(std::make_pair(h, new basicHistosNaming(htitle.Data(), htitle.Data(), hfunc.Data(), hdescr.Data())));

	// 1
	htitle="WG"; hfunc="CSCWireDigi.getWireGroup()"; hdescr="Wire Group for the wire digi";
	h = new TH1F(htitle.Data(), htitle.Data(), 48, -1, 47);
	histosWithDescritions.push_back(std::make_pair(h, new basicHistosNaming(htitle.Data(), htitle.Data(), hfunc.Data(), hdescr.Data())));

	// 2
	htitle="TB"; hfunc="CSCWireDigi.getTimeBin()"; hdescr="Time Bin";
	h = new TH1F(htitle.Data(), htitle.Data(), 18, -1, 17);
	histosWithDescritions.push_back(std::make_pair(h, new basicHistosNaming(htitle.Data(), htitle.Data(), hfunc.Data(), hdescr.Data())));


	if(debuglevel){
		std::cout << " basicAnodeHistos::book         : booking ends with following content in the directory :" << std::endl;
		outdir->Print(); // does not work in CMSSW??!
		for (std::vector<std::pair<TH1F*, basicHistosNaming*>>::iterator it=histosWithDescritions.begin(); it!=histosWithDescritions.end(); ++it)
			(*it).second->Print();
	};
	return true;
};



bool basicAnodeHistos::fillAnodeHistos(const CSCWireDigi& wiredigi){

	if(histosWithDescritions.size()<3) return false;

	histosWithDescritions.at(1).first->Fill(wiredigi.getWireGroup());
	histosWithDescritions.at(2).first->Fill(wiredigi.getTimeBin());
	return true;
};

bool basicAnodeHistos::fillAnodeHistos(short int NclctEvt=-1){
	if(histosWithDescritions.size()<1) return false;

	histosWithDescritions.at(0).first->Fill(NclctEvt);
	return true;
}
