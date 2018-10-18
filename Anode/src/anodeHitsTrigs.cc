#include <iostream>
#include "GifCSC/Anode/interface/anodeHitsTrigs.h"

anodeHitsTrigs::anodeHitsTrigs(short unsigned chamber, edm::EDGetTokenT<CSCALCTDigiCollection> alcttoken, TDirectory *outputroot, short unsigned debuglevel)
					 : chamberType(chamber), commonCurrentEvtN(-1), evtsProceededWGs(0), evtsProccededALCTs(0),
					   debug(debuglevel){
	//std::cout << "here1: " << alct_token.isUninitialized() << "\t" << alct_token.index() << std::endl;
	alct_token = alcttoken;
	//std::cout << "here2: " << alct_token.isUninitialized() << "\t" << alct_token.index() << std::endl;
}

anodeHitsTrigs::anodeHitsTrigs(short unsigned chamber, edm::EDGetTokenT<CSCWireDigiCollection> wiretoken, TDirectory *outputroot, short unsigned debuglevel)
                                         : chamberType(chamber), commonCurrentEvtN(-1), evtsProceededWGs(0), evtsProccededALCTs(0),
                                           debug(debuglevel){
        wires_token = wiretoken;
        wire_histos = NULL; histogramming = false;
        if(outputroot!=NULL){
                if(debug) std::cout << "creating histogram subdirectory in directory " << outputroot->GetPath() << std::endl;
                TDirectory *outdir = outputroot->mkdir("hWire");
                if(outdir->cd()){
                        wire_histos    = new basicAnodeHistos(outdir, debug);
                        histogramming = true;
                }else{std::cout << "Anode can not create directory for basic ALCT historgrams in file " << outputroot->GetPath() << std::endl;}
        }else{
                std::cout << "Anode will not save basic historgrams since no directory was prepared!\n";
        };
}

bool anodeHitsTrigs::end(){
	bool ok = true;
        if(histogramming){
                ok = (ok && wire_histos->write());
        };
        return ok;	
}

anodeHitsTrigs::~anodeHitsTrigs(){
	delete wire_histos;
}

bool anodeHitsTrigs::bookAnodeHistos(){
	bool ok = false;
	return ok;
}
bool anodeHitsTrigs::writeAnodeHistos(){
	bool ok = false;
	return ok;
}
bool anodeHitsTrigs::cleanAnodeHistos(){
	bool ok = false;
	return ok;
}
bool anodeHitsTrigs::alctInEvent(const edm::Event& iEvent){
	edm::Handle<CSCALCTDigiCollection> alct;
	iEvent.getByToken(alct_token, alct);
	if(debug) std::cout << "ALCT handle (anode)" << alct << std::endl;

        short unsigned int   N = 0;
	for (CSCALCTDigiCollection::DigiRangeIterator j=alct->begin(); j!=alct->end(); ++j) {
		CSCDetId id = (CSCDetId)(*j).first;
		if(id.chamber()!=chamberType){
			//std::cout << "chamber id" << id.chamber() << ": chamber type" << chamberType << std::endl; 
			continue;
		}
		if(debug) std::cout   <<  "ALCT from : chamber " << id.chamber()
							  << " station " << id.station()
							  << " ring    " << id.ring()
							  << " layer   " << id.layer() << std::endl;
		std::vector<CSCALCTDigi>::const_iterator digiItr = (*j).second.first;
		std::vector<CSCALCTDigi>::const_iterator last    = (*j).second.second;
		
		while (digiItr != last) {
			if(debug) std::cout << digiItr->getKeyWG()  << "  " << digiItr->isValid() << std::endl;
			++digiItr; N++;
		};
		
	};
	return true;
}

bool anodeHitsTrigs::wiredigiInEvent(const edm::Event& iEvent){
	edm::Handle<CSCWireDigiCollection> wiredigi;
        iEvent.getByToken(wires_token, wiredigi);
        if(debug) std::cout << "WireDigi handle (anode)" << wiredigi << std::endl;

        short unsigned int   N = 0; WG_wiredigi.clear(); TB_wiredigi.clear();

        for (CSCWireDigiCollection::DigiRangeIterator j=wiredigi->begin(); j!=wiredigi->end(); ++j) {
                CSCDetId id = (CSCDetId)(*j).first;
                if(id.chamber()!=chamberType){
			//std::cout << "chamber id" << id.chamber() << ": chamber type" << chamberType << std::endl; 
			continue;
		}
                if(debug) std::cout   <<  "WireDigi from : chamber " << id.chamber()
                                                          << " station " << id.station()
                                                          << " ring    " << id.ring()
                                                          << " layer   " << id.layer() << std::endl;
                std::vector<CSCWireDigi>::const_iterator digiItr = (*j).second.first;
                std::vector<CSCWireDigi>::const_iterator last    = (*j).second.second;
                if(digiItr!=last) evtsProceededWGs++;
                while (digiItr != last) {
			if(debug){
				std::cout << "WG: " << digiItr->getWireGroup()  << "  CSC Wire First Time Bin On: " << digiItr->getTimeBin() << std::endl;
				WG_wiredigi.push_back(digiItr->getWireGroup());
				TB_wiredigi.push_back(digiItr->getTimeBin());
                             	std::cout << " CSC Time Bins On: " ;
				for (unsigned int i = 0; i<digiItr->getTimeBinsOn().size(); ++i ){
					std::cout << "," << digiItr->getTimeBinsOn()[i]; }
				std::cout << std::endl;
			}
                        WG_wiredigi.push_back(digiItr->getWireGroup());
                        TB_wiredigi.push_back(digiItr->getTimeBin());
                        if(histogramming) wire_histos->fillAnodeHistos(*digiItr);
                        ++digiItr; N++;
                };
        };
	if(histogramming) wire_histos->fillAnodeHistos(N);
        return true;
}





