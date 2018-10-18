#include "GifCSC/Cathode/interface/cathodeHitsTrigs.h"
#include "GifCSC/Cathode/interface/cathodePrintOuts.h"

#include <iostream>

cathodeHitsTrigs::cathodeHitsTrigs(short unsigned     chamber,
		edm::EDGetTokenT<CSCCLCTDigiCollection>       clcttoken,
		edm::EDGetTokenT<CSCComparatorDigiCollection> comptoken,
		TDirectory *                                  outputroot,
		short unsigned debuglevel)
					 : chamberType(chamber), commonCurrentEvtN(-1), evtsProccededCLCTs(0),
					   debug(debuglevel){
	//CLCT
	clct_token  = clcttoken;
	//comparators
	comp_token  = comptoken;

	// histos
	clct_histos = NULL; histogramming = false;
	if(outputroot!=NULL){
		if(debug) std::cout << "creating histogram subdirectory in directory " << outputroot->GetPath() << std::endl;
		TDirectory *outdir = outputroot->mkdir("hCLCT");
		if(outdir->cd()){
			clct_histos   = new basicCLCTHistos(outdir, debug);
			histogramming = true;
		}else{std::cout << "cathodeHitsTrigs can not create directory for basic CLCT historgrams in file " << outputroot->GetPath() << std::endl;}
	}else{
		std::cout << "cathodeHitsTrigs will not save basic historgrams since no directory was prepared!\n";
	};
}


bool cathodeHitsTrigs::end(){
	bool ok = true;
	if(histogramming){
		ok = (ok && clct_histos->write());
	};
	return ok;
}



cathodeHitsTrigs::~cathodeHitsTrigs(){
	delete clct_histos; // histos should be already written
}



bool cathodeHitsTrigs::clctInEvent(const edm::Event& iEvent){
//http://cmsdoxygen.web.cern.ch/cmsdoxygen/CMSSW_7_5_1/doc/html/d5/d6e/CSCCLCTDigiCollection_8h_source.html
//typedef MuonDigiCollection<CSCDetId, CSCCLCTDigi> CSCCLCTDigiCollection;
//http://cmsdoxygen.web.cern.ch/cmsdoxygen/CMSSW_7_5_1/doc/html/da/d26/classCSCCLCTDigi.html
	bool ok = false;
	edm::Handle<CSCCLCTDigiCollection> clct;
	iEvent.getByToken(clct_token, clct);

	short unsigned int   N = 0;
	short unsigned int dBX = 0;
	for (CSCCLCTDigiCollection::DigiRangeIterator clctit=clct->begin(); clctit!=clct->end(); ++clctit) {
		CSCDetId id = (CSCDetId)(*clctit).first;
		if(id.chamber()!=chamberType) continue;
		if(debug) std::cout   <<  "CLCT from : chamber " << id.chamber()
							  << " station " << id.station()
							  << " ring    " << id.ring()
							  << " layer   " << id.layer() << std::endl;
		std::vector<CSCCLCTDigi>::const_iterator digiItr = (*clctit).second.first;
		std::vector<CSCCLCTDigi>::const_iterator last    = (*clctit).second.second;
		if(digiItr!=last) evtsProccededCLCTs++;
		short int       lastBX = -100;
		while (digiItr != last) {
			if(debug)          CLCT_print(*digiItr);
			if(histogramming)  clct_histos->fillCLCTHistos(*digiItr);

			short int bx = (*digiItr).getBX();
			if(N>0 and bx!=lastBX)
				if(abs(bx-lastBX)<dBX) dBX=lastBX;

			lastBX = bx;
			++digiItr; N++;
		};
	}; // end loop CSCCLCTDigiCollection
	if(histogramming) clct_histos->fillCLCTHistos(N, dBX);
	return ok;
};



bool cathodeHitsTrigs::compInEvent(const edm::Event& iEvent){
	//https://github.com/cms-sw/cmssw/blob/CMSSW_7_5_X/DataFormats/CSCDigi/interface/CSCComparatorDigi.h
	bool ok = false;
	edm::Handle<CSCComparatorDigiCollection> comp;
	iEvent.getByToken(comp_token, comp);

	if(debug)std::cout << "cathodeHitsTrig::compInEvent\n chamber layer getStrip() getComparator\n";
	for (CSCComparatorDigiCollection::DigiRangeIterator compit=comp->begin(); compit!=comp->end(); ++compit) {
		CSCDetId id = (CSCDetId)(*compit).first;
		if(id.chamber()!=chamberType) continue;
		std::vector<CSCComparatorDigi>::const_iterator digiItr = (*compit).second.first;
		std::vector<CSCComparatorDigi>::const_iterator last          = (*compit).second.second;
		if(debug){
			while (digiItr != last) {
				std::cout << id.chamber() <<"  "<< id.layer()<< "  "<< digiItr->getStrip()  << "  " <<  digiItr->getComparator() << std::endl;
				digiItr->print();
				++digiItr;
			};
		};
	}; // end loop Collection

	return ok;
};


