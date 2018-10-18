// -*- C++ -*-
//
// Package:    GifCSC/GIFanlzr
// Class:      GIFanlzr
// 
/**\class GIFanlzr GIFanlzr.cc GifCSC/GIFanlzr/plugins/GIFanlzr.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Katerina Kuznetsova
//         Created:  Mon, 06 Nov 2017 13:21:13 GMT
//
//


// system include files
#include <memory>
#include <vector>
#include <iostream>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "TFile.h"

#include "GifCSC/Anode/interface/anodeHitsTrigs.h"
#include "GifCSC/Segment/interface/Segment.h"
#include "GifCSC/Cathode/interface/cathodeHitsTrigs.h"
#include "GifCSC/LCT/interface/LCT.h"
#include "GifCSC/StatusDigis/interface/StatusDigis.h"

//
// class declaration
//

// If the analyzer does not use TFileService, please remove
// the template argument to the base class so the class inherits
// from  edm::one::EDAnalyzer<> and also remove the line from
// constructor "usesResource("TFileService");"
// This will improve performance in multithreaded jobs.

class GIFanlzr : public edm::one::EDAnalyzer<edm::one::SharedResources>  {
   public:
      explicit GIFanlzr(const edm::ParameterSet&);
      ~GIFanlzr();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);


   private:
      virtual void beginJob() override;
      virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
      virtual void endJob() override;
      // ----------member data ---------------------------
      TFile * outputfile = NULL;
      unsigned int chamberType;  // 0 - both; 1 - ME1/1; 2 - ME2/1; >10 - P5 (not yet)
      unsigned int anodeTag;     // 0 - none; 1 - proceed;
      unsigned int segmentTag;   // 0 - none; 1 - proceed;
      unsigned int cathodeTag;   // 0 - none; 1 - proceed;
      unsigned int lctTag;       // 0 - none; 1 - proceed;
      unsigned int tmbTag;       // 0 - none; 1 - proceed;
      unsigned int statusTag;    // 0 - none; any of status tags above
      unsigned int debugTag;     // 0 - no debug, 1 analyzer, 2 all
      unsigned int NofChambers;
      std::vector <anodeHitsTrigs*>   chamberwiseAnodeInfo;
      std::vector <Segment*>     chamberwiseSegmentInfo;
      std::vector <cathodeHitsTrigs*> chamberwiseCathodeInfo;
      std::vector <LCT*>              chamberwiseLCTInfo;
      std::vector <StatusDigis*>      chamberwiseStatusInfo;
      edm::EDGetTokenT<CSCCLCTDigiCollection>          clct_token;
      edm::EDGetTokenT<CSCWireDigiCollection>          wires_token;
      edm::EDGetTokenT<CSCComparatorDigiCollection>    comp_token;
      edm::EDGetTokenT<CSCALCTDigiCollection>          alct_token;
      edm::EDGetTokenT<CSCCorrelatedLCTDigiCollection> lct_token;
      edm::EDGetTokenT<CSCSegmentCollection>           segment_token;
      edm::EDGetTokenT<CSCTMBStatusDigiCollection>     tmb_token;
      ///!!!TBD!!! here should be a struct fo tokens for StatusDigis
      TH1F *h_seg_excl = new TH1F("h_exclWG", "Segment WG excluded region", 49, -1, 48);
      TH2F *h_wire_in  = new TH2F("h_wirein", "WG inside excluded region", 48, 0, 48, 16, 0, 16);
      TH2F *h_wire_out = new TH2F("h_wireou", "WG outside excluded region", 48, 0, 48, 16, 0, 16);
      TDirectory *anlzdir;
      //more status collections here
};





GIFanlzr::GIFanlzr(const edm::ParameterSet& iConfig){
	usesResource("TFileService");
	chamberType  = iConfig.getUntrackedParameter<int>("chamberTag");

	anodeTag                    = iConfig.getUntrackedParameter<int>("anodeTag");
	if( anodeTag ){ 
			alct_token   = consumes<CSCALCTDigiCollection>(iConfig.getParameter<edm::InputTag>("alctDigiTag"));
			wires_token  = consumes<CSCWireDigiCollection>(iConfig.getParameter<edm::InputTag>("wireDigiTag"));
	}

        segmentTag                    = iConfig.getUntrackedParameter<int>("segmentTag");
        if( segmentTag ){
                        segment_token   = consumes<CSCSegmentCollection>(iConfig.getParameter<edm::InputTag>("cscSegTag"));
        }

	cathodeTag                  = iConfig.getUntrackedParameter<int>("cathodeTag");
	if( cathodeTag){
					clct_token  = consumes<CSCCLCTDigiCollection>(iConfig.getParameter<edm::InputTag>("clctDigiTag"));
					comp_token  = consumes<CSCComparatorDigiCollection>(iConfig.getParameter<edm::InputTag>("comparatorDigiTag"));
	};

	lctTag                      = iConfig.getUntrackedParameter<int>("lctTag");
	if( lctTag )   lct_token    = consumes<CSCCorrelatedLCTDigiCollection>(iConfig.getParameter<edm::InputTag>("lctDigiTag"));

	//StatusDigi
	tmbTag                      = iConfig.getUntrackedParameter<int>("tmbTag");
	if( tmbTag)	tmb_token       = consumes<CSCTMBStatusDigiCollection>(iConfig.getParameter<edm::InputTag>("tmbStatusDigiTag"));

	statusTag = (tmbTag);
	//!!!TBD!!! fill the structure of status tokens

    //debug level
	debugTag     = iConfig.getUntrackedParameter<int>("debugTag");
	int inDL = 0; // inner debug level
	if (debugTag>0) inDL=debugTag-1;


	short unsigned int chtmin = 1;
	short unsigned int chtmax = 2;
	NofChambers = 2;
	if(chamberType>0 and chamberType<3) {
		chtmin=chtmax=chamberType;
		NofChambers=1;
	}else if(chamberType>=3) {
		std::cout << "Unknown chamber type " << chamberType << "exiting...\n";
		exit(11);
	};


	// !!!TBD!!! (1)name from config
	outputfile   = new TFile("output.root","RECREATE"); // our NULL
	TDirectory * outdir = NULL;

	// loop over chambers
	for(short unsigned int i=chtmin; i<=chtmax; i++){

		if(outputfile){
			TString chname="ME"; chname+=i; chname+="1";
			outputfile->cd(); outdir = outputfile->mkdir(chname.Data());
                 	anlzdir = outdir->mkdir("anlz");
		};

		if(anodeTag)    chamberwiseAnodeInfo.push_back  (new anodeHitsTrigs  (i, wires_token, outdir, inDL));
                if(segmentTag)  chamberwiseSegmentInfo.push_back(new Segment    (i, segment_token, outdir, inDL));
		if(cathodeTag)  chamberwiseCathodeInfo.push_back(new cathodeHitsTrigs(i, clct_token, comp_token, outdir, inDL));
		if(lctTag)      chamberwiseLCTInfo.push_back    (new LCT             (i, lct_token, outdir, inDL));

		// !!!TBD!!! should be a structure of tokens!!! currently only tmb_token
		if(statusTag) 	chamberwiseStatusInfo.push_back(new StatusDigis(i, tmb_token, outdir, inDL));

	}; // end loop over chambers
 }



GIFanlzr::~GIFanlzr(){

	if(anodeTag>0)
		for(std::vector<anodeHitsTrigs*>::iterator aii = chamberwiseAnodeInfo.begin(); aii!=chamberwiseAnodeInfo.end(); ++aii)
			delete *aii;
        if(segmentTag>0)
                for(std::vector<Segment*>::iterator segii = chamberwiseSegmentInfo.begin(); segii!=chamberwiseSegmentInfo.end(); ++segii)
                        delete *segii;
	if(cathodeTag>0)
		for(std::vector<cathodeHitsTrigs*>::iterator cii = chamberwiseCathodeInfo.begin(); cii!=chamberwiseCathodeInfo.end(); ++cii)
			delete *cii;
	if(lctTag>0)
		for(std::vector<LCT*>::iterator lii = chamberwiseLCTInfo.begin(); lii!=chamberwiseLCTInfo.end(); ++lii)
			delete *lii;
	if(statusTag>0)
		for(std::vector<StatusDigis*>::iterator sii = chamberwiseStatusInfo.begin(); sii!=chamberwiseStatusInfo.end(); ++sii)
			delete *sii;

	if(outputfile) outputfile->Close();
}




void GIFanlzr::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup){
	for(short unsigned int i=0; i<NofChambers; i++){
		if( anodeTag   ) chamberwiseAnodeInfo.at(i)->wiredigiInEvent(iEvent);
 		if( segmentTag ) chamberwiseSegmentInfo.at(i)->segmentInEvent(iEvent);
		if( cathodeTag ) {
			chamberwiseCathodeInfo.at(i)->clctInEvent(iEvent);
			chamberwiseCathodeInfo.at(i)->compInEvent(iEvent);
		};
		if( lctTag )    chamberwiseLCTInfo.at(i)->lctInEvent(iEvent);
		if( statusTag ) {
			if( tmbTag) chamberwiseStatusInfo.at(i)->tmbStatInEvent(iEvent);
		};

		if(anodeTag&&segmentTag){
                        if(chamberwiseSegmentInfo.at(i)->GetNperEvt() != 1) continue;
                        std::vector<int> WG_Seg = chamberwiseSegmentInfo.at(i)->GetWG();
                        std::vector<int> WG_wiredigi = chamberwiseAnodeInfo.at(i)->GetWG();
                        std::vector<int> TB_wiredigi = chamberwiseAnodeInfo.at(i)->GetTB();

			std::sort(WG_Seg.begin(), WG_Seg.end());
			WG_Seg.push_back(WG_Seg.back()+1); WG_Seg.push_back(WG_Seg.back()+1); WG_Seg.push_back(WG_Seg.front()-1); WG_Seg.push_back(WG_Seg.front()-2);
			std::sort(WG_Seg.begin(), WG_Seg.end());
			//std::cout << "****" << WG_Seg[0] << "***" << WG_wiredigi[0] << "**" << TB_wiredigi[0] << std::endl;
                        //if(WG_Seg.size()==0) { std::cerr << "0 WG in Segments" << endl; exit(0); }
			//if(WG_wiredigi.size()==0) {std::cout << "0 WG in WireDigi" << std::endl; continue; }

			for(unsigned int i = 0; i < WG_Seg.size(); i++) { h_seg_excl->Fill(WG_Seg[i]); }
			for(unsigned int i = 0; i < WG_wiredigi.size(); i++) {
				if( (WG_wiredigi[i] > WG_Seg.front()) && (WG_wiredigi[i] < WG_Seg.back()) ) { h_wire_in->Fill(WG_wiredigi[i], TB_wiredigi[i]); }
				else{ h_wire_out->Fill(WG_wiredigi[i], TB_wiredigi[i]); }
			}

		}


	}

	if(debugTag) std::cout << "\n\n";
}





void GIFanlzr::beginJob(){

	anlzdir->cd();

}


void GIFanlzr::endJob() {
	std::cout << "GIFanlzr::endJob()" << std::endl;
	// zachem ja eto delala?! 8)
	for(short unsigned int i=0; i<NofChambers; i++){
		//chamberwiseCathodeInfo.at(i)->end();
		//chamberwiseLCTInfo.at(i)->end();
		chamberwiseAnodeInfo.at(i)->end();
		chamberwiseSegmentInfo.at(i)->end();
                anlzdir->cd();
		h_seg_excl->Write(); h_wire_in->Write(); h_wire_out->Write();
		anlzdir->Write();
	}

}

void GIFanlzr::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(GIFanlzr);
