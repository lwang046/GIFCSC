#include <utility>
#include <iostream>

#include "TDirectory.h"
#include "TH1F.h"

#include "GifCSC/Cathode/interface/cathodeHistos.h"
#include "GifCSC/basics/interface/basicHistosNaming.h"



basicCLCTHistos::basicCLCTHistos(TDirectory * dout, const int debug):basicHistos(dout, debug){
	if(debuglevel>1){
		std::cout << " basicCLCTHistos::basicCLCTHistos   : directory pointer " << &outdir << "; directory name: ";
		std::cout << outdir->GetPath() << std::endl;
	};
	book();
	if(debuglevel)std::cout << " basicCLCTHistos::basicCLCTHistos	   : => completed " << std::endl;
}



bool basicCLCTHistos::book(){
	outdir->cd(); // important
	TH1F * h;
	TString htitle, hdescr, hfunc;

	// 0
	htitle="NperEvt"; hfunc="evaluated"; hdescr="number of CLCTs per event";
	h = new TH1F(htitle.Data(), htitle.Data(), 11, -1, 10);
	histosWithDescritions.push_back(std::make_pair(h, new basicHistosNaming(htitle.Data(), htitle.Data(), hfunc.Data(), hdescr.Data())));

	// 1
	htitle="dBX"; hfunc="evaluated"; hdescr="minDifference between two CLCTs from different BX";
	h = new TH1F(htitle.Data(), htitle.Data(), 11, -1, 10);
	histosWithDescritions.push_back(std::make_pair(h, new basicHistosNaming(htitle.Data(), htitle.Data(), hfunc.Data(), hdescr.Data())));

	// 2
	htitle="validity"; hfunc="CSCCLCTDigi.isValid()"; hdescr="validity";
	h = new TH1F(htitle.Data(), htitle.Data(), 3, -1, 2);
	histosWithDescritions.push_back(std::make_pair(h, new basicHistosNaming(htitle.Data(), htitle.Data(), hfunc.Data(), hdescr.Data())));

	// 3
	htitle="halfstripN";  hfunc="CSCCLCTDigi.getKeyStrip()"; hdescr="absolute halfstrip number";
	h = new TH1F(htitle.Data(), htitle.Data(), 251, -1, 250);
	histosWithDescritions.push_back(std::make_pair(h, new basicHistosNaming(htitle.Data(), htitle.Data(), hfunc.Data(), hdescr.Data())));

	// 4
	htitle="quality";  hfunc="CSCCLCTDigi.getQuality()"; hdescr="quality";
	h = new TH1F(htitle.Data(),htitle.Data(), 101, -1, 100);
	histosWithDescritions.push_back(std::make_pair(h, new basicHistosNaming(htitle.Data(), htitle.Data(), hfunc.Data(), hdescr.Data())));

	// 5
	htitle="pattern";  hfunc="CSCCLCTDigi.getPattern()"; hdescr="pattern";
	h = new TH1F(htitle.Data(),htitle.Data(), 21, -1, 20);
	histosWithDescritions.push_back(std::make_pair(h, new basicHistosNaming(htitle.Data(), htitle.Data(), hfunc.Data(), hdescr.Data())));

	// 6
	htitle="BX";  hfunc="CSCCLCTDigi.getBX()"; hdescr="relative BX number";
	h = new TH1F(htitle.Data(),htitle.Data(), 20, -10, 10);
	histosWithDescritions.push_back(std::make_pair(h, new basicHistosNaming(htitle.Data(), htitle.Data(), hfunc.Data(), hdescr.Data())));

	if(debuglevel){
		std::cout << " basicCLCTHistos::book         : booking ends with following content in the directory :" << std::endl;
		outdir->Print(); // does not work in CMSSW??!
		for (std::vector<std::pair<TH1F*, basicHistosNaming*>>::iterator it=histosWithDescritions.begin(); it!=histosWithDescritions.end(); ++it)
			(*it).second->Print();
	};
	return true;
};



bool basicCLCTHistos::fillCLCTHistos(const CSCCLCTDigi& clctdigi){

	if(histosWithDescritions.size()<7) return false;

	histosWithDescritions.at(2).first->Fill(clctdigi.isValid());
	histosWithDescritions.at(3).first->Fill(clctdigi.getKeyStrip());
	histosWithDescritions.at(4).first->Fill(clctdigi.getQuality());
	histosWithDescritions.at(5).first->Fill(clctdigi.getPattern());
	histosWithDescritions.at(6).first->Fill(clctdigi.getBX());
	return true;
};

bool basicCLCTHistos::fillCLCTHistos(short int NclctEvt=-1, short int dBX=-1){
	if(histosWithDescritions.size()<2) return false;

	histosWithDescritions.at(0).first->Fill(NclctEvt);
	histosWithDescritions.at(1).first->Fill(dBX);
	return true;
}
