// -*- C++ -*-
//
// Package:    test/gifRates
// Class:      gifRates
// 
/**\class gifRates gifRates.cc test/gifRates/plugins/gifRates.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Katerina Kuznetsova
//         Created:  Fri, 15 Jul 2016 10:01:03 GMT
//
//


// system include files
#include <DataFormats/CSCRecHit/interface/CSCRecHit2D.h>
#include <DataFormats/CSCRecHit/interface/CSCSegmentCollection.h>
#include <DataFormats/CSCDigi/interface/CSCALCTDigi.h>
#include <DataFormats/CSCDigi/interface/CSCALCTDigiCollection.h>

#include <FWCore/Framework/interface/ESHandle.h>
#include <Geometry/Records/interface/MuonGeometryRecord.h>
#include <iostream>
#include <memory>

#include "DataFormats/CSCDigi/interface/CSCWireDigi.h"
#include "DataFormats/CSCDigi/interface/CSCWireDigiCollection.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "Geometry/CSCGeometry/interface/CSCChamber.h"
#include "Geometry/CSCGeometry/interface/CSCGeometry.h"
#include "Geometry/CSCGeometry/interface/CSCLayer.h"
#include "Geometry/CSCGeometry/interface/CSCLayerGeometry.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TString.h"


//===========================================================================
//===========================================================================
// returns:
// 10 - wrong parameter

class gifRates : public edm::one::EDAnalyzer<edm::one::SharedResources>  {

public:
  explicit gifRates(const edm::ParameterSet&);
  ~gifRates();  
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
  
private:
  virtual void beginJob() override;
  virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
  virtual void endJob() override;

  // input parameters/tokens:
  edm::EDGetTokenT<CSCWireDigiCollection> wires_token;
  edm::EDGetTokenT<CSCALCTDigiCollection> alct_token;

  edm::InputTag cscRecHitTag;
  edm::InputTag cscSegTag;
  bool          anodeOnlyTag; //if true (test11) wNorm = 1 else should account for missing CFEBs


  // number of events proceeded:
  int evtsProceededA; // event counter anode hits
  int evtsProceeded2D; // event counter anode hits
  int evtsProceededS; // event counter segments

  int prevEvtN;

  float * wL;        // WG length array
  float * wN;        // Number of wires per a wiregroup
  float * wH1;       // array with wirehits (L1)
  float wNorm;       // normalization factor for wire occupancies (test11 vs test27s)
  bool    fillWL;    // flag to fill wirelength array, true by constructor, false after filling

  bool                       debug;
  int                        chamberType;
  const short unsigned int * nOfWGs;

  edm::ESHandle<CSCGeometry> h;
  
  void handleAnodes  (const edm::Handle<CSCWireDigiCollection> wires, const edm::Handle<CSCALCTDigiCollection> alct);
  void handle2Dhits  (const edm::Handle<CSCRecHit2DCollection> rechits);
  int handleSegments (const edm::Handle<CSCSegmentCollection> segments);
  
  TFile * fout;     // file to save root histis

  TH1F  * hNofAhit[3];     // number of anode hits from CSCWireDigi per event [0] - all hits, [1] - "single" hits, [3] - number of fired layers
  TH1F  * hNofAhitL[6][3]; // number of anode hits from CSCWireDigi per layer per event [0] - all hits, [1] - "single" hits, [3] - fired layer
  TH1F  * hAhitL[6][2];    // anode hits from WGs: [0] - all hits, [1] - "single" hits
  TH1F  * hAhitLNorm[6][2];// anode hits from WGs normalized to length of WGs (wL): [0] - all hits, [1] - "single" hits
  TH2F  * h2dNofAhitWG[6];   // number of "simultaneous" hits from a WG (first WG is assigned to be actual one)
  TH2F  * hALength1;       // anode wire length

  TH1F  * hNofRhit;       // number of RecHits per event
  TH1F  * hNofRhitL[6];    // number of RecHits per layer per event
  TH2F  * h2dRHL[6];       // 2D histos of RecHits from csc2DRecHits

  TH2F  * h2dSeg;       // 2D histos of segments belonging to CSCSegmentCollection
  TH2F  * h2dSeg4;       // 2D histos of segments belonging to CSCSegmentCollection
  TH1F  * hNofSeg;      // number of segments per event
  TH1F  * hNofSeg4;      // number of segments per event
  TH1F  * hsegChi2;     // chi2
  TH1F  * hsegChi24;     // chi2
  TH1F  * hsegNof2Dhits;
  TH1F  * hsegNof2Dhits4;
  
  /// !!! TBD to move to header of separate object
  const short unsigned int nOfWGs1 = 48;
  const short unsigned int nOfWGs2 = 112;



};


//===========================================================================
//===========================================================================


gifRates::gifRates(const edm::ParameterSet& iConfig):
		evtsProceededA(0),
		evtsProceeded2D(0),
		evtsProceededS(0),
		prevEvtN(0),
		wNorm(1.0),
		fillWL(true){
   usesResource("TFileService");

   edm::InputTag wireDigiTag  = iConfig.getParameter<edm::InputTag>("wireDigiTag");
   wires_token = consumes<CSCWireDigiCollection>(wireDigiTag);

   edm::InputTag alctDigiTag  = iConfig.getParameter<edm::InputTag>("alctDigiTag");
   alct_token = consumes<CSCALCTDigiCollection>(alctDigiTag);


   cscRecHitTag = iConfig.getParameter<edm::InputTag>("cscRecHitTag");
   cscSegTag    = iConfig.getParameter<edm::InputTag>("cscSegTag");
   chamberType  = iConfig.getUntrackedParameter<int>("chamberTag");
   anodeOnlyTag = iConfig.getUntrackedParameter<bool>("anodeOnlyTag");
   debug        = iConfig.getUntrackedParameter<bool>("debugTag");

   std::cout << "plotting for chamber type " << chamberType << std::endl;
   
   /// !!! TBD fine normalization for ME21 (https://twiki.cern.ch/twiki/pub/CMS/CSCDPGGeometry/csc_wire_groups_100928.pdf)
   switch (chamberType){
	   case 1:
		 nOfWGs = &nOfWGs1;
		 if(!anodeOnlyTag) wNorm = 0.75;
		 break;

	   case 2:
		 nOfWGs = &nOfWGs2;
		 if(!anodeOnlyTag) wNorm = 0.8;
		 break;

	   default:
		 std::cout << "Unknown chamber type: " << chamberType << "exiting\n";
		 exit(10);
		 break;
   };

   wL = new float [*nOfWGs];
   wN = new float [*nOfWGs];
   wH1 = new float [*nOfWGs];
   for(short unsigned int i=0; i<*(nOfWGs);i++){
	   wL[i]  = 0;
	   wN[i]  = 0;
	   wH1[i] = 0;
   };
}



gifRates::~gifRates(){
	delete wL;
	delete wH1;
}



/*****************************************************
* creates output histograms at the beginning of job  *
******************************************************/
void gifRates::beginJob(){
  fout    = new TFile("histos.root", "RECREATE" );
  int nbins = *(nOfWGs)+2;
  //std::cout << "nbins " << nbins << std::endl;
  TString hname;

  fout->mkdir("ALCT");
  hALength1    = new TH2F("hALength1",    "hALength1",   nbins, -1, nbins+1, 500,  0,   500);
  hNofAhit[0]  = new TH1F("hNofAhit[0]",   "hNofAhit[0]", 41,   -1.5, 39.5);
  hNofAhit[1]  = new TH1F("hNofAhit[1]",   "hNofAhit[1]", 41,   -1.5, 39.5);
  hNofAhit[2]  = new TH1F("hNofAhit[2]",   "hNofAhit[2]", 41,   -1.5, 39.5);
  if(!anodeOnlyTag){
	  fout->mkdir("2dRH");
	  hNofRhit     = new TH1F("hNofRhit",     "hNofRhit", 41,   -1.5, 39.5);
  };

  for(short unsigned int i=0; i<6; i++){
	  fout->cd("ALCT");
	  for(short unsigned int j=0; j<3; j++){
		  hname = "hNofAhitL[";   hname+=i; hname+="]["; hname+=j; hname+="]"; hNofAhitL[i][j] = new TH1F(hname.Data(), hname.Data(), 41,   -1.5, 39.5);
	  };
	  for(short unsigned int j=0; j<2; j++){
		  hname = "hAhitL[";      hname+=i; hname+="]["; hname+=j; hname+="]"; hAhitL[i][j]    = new TH1F(hname.Data(), hname.Data(), nbins, 0, nbins);
		  hname = "hAhitLNorm[";  hname+=i; hname+="]["; hname+=j; hname+="]"; hAhitLNorm[i][j]= new TH1F(hname.Data(), hname.Data(), nbins, 0, nbins);
	  };
	  hname = "h2dNofAhitWG[";hname+=i; hname+="]"; h2dNofAhitWG[i] = new TH2F(hname.Data(), hname.Data(), nbins, 0, nbins, 11, -1.5, 9);
	  if(!anodeOnlyTag){
		  fout->cd("2dRH");
		  hname = "hNofRhitL[";  hname+=i; hname+="]"; hNofRhitL[i] = new TH1F(hname.Data(), hname.Data(), 41,   -1.5, 39.5);
		  hname = "h2dRHL[";     hname+=i; hname+="];localY [cm];-localX [cm]";
		  h2dRHL[i]     = new TH2F(hname.Data(), hname.Data(), 500, -150, 150, 500, -100, 100);
	  };
  };

  if(!anodeOnlyTag){
	  fout->mkdir("Segments");
	  fout->cd("Segments");
	  h2dSeg       = new TH2F("h2dSeg",      "h2dSeg;localY [cm];-localX [cm]", 500, -150, 150, 500, -100, 100);
	  h2dSeg4      = new TH2F("h2dSeg4",     "h2dSeg;localY [cm];-localX [cm]", 500, -150, 150, 500, -100, 100);
	  hNofSeg      = new TH1F("hNofSeg",      "hNofSeg;number of segments",      21,   -1,  20);
	  hsegChi2     = new TH1F("hsegChi2",     "hsegChi2;#{Chi}^{2} of segment",  1000,  0,  20);
	  hsegNof2Dhits= new TH1F("hsegNof2Dhits","hsegNof2Dhits; number of 2D RecHit per segment", 10, -1, 9);
	  hNofSeg4      = new TH1F("hNofSeg4",      "hNofSeg4;number of segments",      21,   -1,  20);
	  hsegChi24     = new TH1F("hsegChi24",     "hsegChi24;#{Chi}^{2} of segment",  1000,  0,  20);
	  hsegNof2Dhits4= new TH1F("hsegNof2Dhits4","hsegNof2Dhits4; number of 2D RecHit per segment", 10, -1, 9);
  };
}




/************************************************************
* normalises and saves output histograms at the end of job  *
*************************************************************/
void gifRates::endJob() {
	float WGtotalWireLength[*nOfWGs];
	//https://twiki.cern.ch/twiki/pub/CMS/CSCagingTest/Kamenev-ME1-1_GIF_Wirelength.pdf
	float ME11GIFwgTotalWL[48] = {
			 2901.9863  , 2995.38356 , 3088.78082 , 2948.68493 , 3029.21624 , 3109.74755 , 3190.27886 , 3270.81018 ,
			 3351.34149 , 3431.8728  , 3512.40411 , 3592.93542 , 3673.46673 , 3753.99804 , 3834.52935 , 3915.06067 ,
			 3682.52055 , 3751.13894 , 3819.75734 , 3888.37573 , 3956.99413 , 4025.61252 , 4094.23092 , 4162.84932 ,
			 4231.46771 , 4300.08611 , 4368.7045  , 4437.3229  , 4505.94129 , 4574.55969 , 4643.17808 , 4711.79648 ,
			 4780.41487 , 4849.03327 , 4917.65166 , 4986.27006 , 5054.88845 , 5123.50685 , 5192.12524 , 5260.74364 ,
			 5329.36204 , 5397.98043 , 5466.59883 , 5535.21722 , 5603.83562 , 5672.45401 , 5741.07241 , 5809.6908
	};
	for(short unsigned int i=0; i<*(nOfWGs);i++){
		if(chamberType==1){ // currently GIF type
			WGtotalWireLength[i] = ME11GIFwgTotalWL[i]/10;
		}else if (chamberType==2){
			WGtotalWireLength[i] = wL[i]*wN[i];
		}else{
			WGtotalWireLength[i] = 1;
		};
	};
	if(evtsProceededA>0){
		double norm = evtsProceededA;
		for(short unsigned int l=0; l<6; l++){
			for(short unsigned int i=0; i<*(nOfWGs);i++){
				if(wL[i]>0){
					if( !(chamberType==1 && i<14) ){ // if not ME11A where all DCFEBs are installed
						norm *= wNorm;
					};
					for(short unsigned int j=0; j<2; j++){
						//hAhitLNorm[l][j]->SetBinContent(i+1, hAhitL[l][j]->GetBinContent(i+1)/wL[i]/norm);
						//hAhitLNorm[l][j]->SetBinError  (i+1, hAhitL[l][j]->GetBinError(i+1)/wL[i]/norm);
						hAhitLNorm[l][j]->SetBinContent(i+1, hAhitL[l][j]->GetBinContent(i+1)/WGtotalWireLength[i]/norm);
						hAhitLNorm[l][j]->SetBinError  (i+1, hAhitL[l][j]->GetBinError(i+1)/WGtotalWireLength[i]/norm);

					};
				}else{
					for(short unsigned int j=0; j<2; j++)
						hAhitLNorm[l][j]->SetBinContent(i+1, 1);

				};
			};
		}; // end layer loop
	};


	if(!anodeOnlyTag && (evtsProceededS>0)){
	  hNofSeg->Sumw2();
	  hNofSeg->Scale(1./hNofSeg->GetEntries());
	};

	for(short unsigned int l=0; l<6; l++){
		fout->cd("ALCT");
		for(short unsigned int j=0; j<3; j++){
			hNofAhitL[l][j]->Write();
			std::cout << hNofAhitL[l][j]->GetName() << "\t" << hNofAhitL[l][j]->GetEntries() << std::endl;
		};
		for(short unsigned int j=0; j<2; j++){
			hAhitL[l][j]->Write();
			std::cout << hAhitL[l][j]->GetName() << "\t" << hAhitL[l][j]->GetEntries() << std::endl;
			hAhitLNorm[l][j]->Write();
		};
		h2dNofAhitWG[l]->Write();
		std::cout << h2dNofAhitWG[l]->GetName() << "\t" << h2dNofAhitWG[l]->GetEntries() << std::endl;
		if(!anodeOnlyTag){
			fout->cd("2dRH");
			hNofRhitL[l]->Write();
			h2dRHL[l]->Write();
		};
	};

	fout->cd("ALCT");
	for(short unsigned int j=0; j<3; j++){
		hNofAhit[j]->Write();
		std::cout << hNofAhit[j]->GetName() << "\t" << hNofAhit[j]->GetEntries() << std::endl;
	};
	hALength1->Write();

	if(!anodeOnlyTag){
		fout->cd("2dRH");
		hNofRhit->Write();

		fout->cd("Segments");
		h2dSeg->Write();
		hNofSeg->Write();
		hsegChi2->Write();
		hsegNof2Dhits->Write();
		h2dSeg4->Write();
		hNofSeg4->Write();
		hsegChi24->Write();
		hsegNof2Dhits4->Write();
	};
	fout->Close();

	for(short unsigned int i=0; i<6; i++){
		for(short unsigned int j=0; j<3; j++){
			hNofAhitL[i][j]->Delete();
		};
		for(short unsigned int j=0; j<2; j++){
			hAhitL[i][j]->Delete();
			hAhitLNorm[i][j]->Delete();
		};
		h2dNofAhitWG[i]->Delete();
		if(!anodeOnlyTag){
			hNofRhitL[i]->Delete();
			h2dRHL[i]->Delete();
		};
	};
	for(short unsigned int j=0; j<3; j++)
		hNofAhit[j]->Delete();
	hALength1->Delete();

	if(!anodeOnlyTag){
		h2dSeg->Delete();
		hNofSeg->Delete();
		hsegChi2->Delete();
		hsegNof2Dhits->Delete();
		h2dSeg4->Delete();
		hNofSeg4->Delete();
		hsegChi24->Delete();
		hsegNof2Dhits4->Delete();
	}

	float sumWL    = 0;
	float sumWLxWN = 0;
	for(short unsigned int i=0; i<*(nOfWGs);i++){
		sumWL+=   wL[i];
		sumWLxWN+=wL[i]*wN[i];
		std::cout << i<< "\t"<< wL[i] << "\t"<< wN[i] << "\t"<< wL[i]*wN[i] << "\t" << wH1[i] << std::endl;
	};
	std::cout << "total wireLength " << sumWL << "; with wN " << sumWLxWN << std::endl;
	std::cout << "total events:    " << prevEvtN       << std::endl;
	std::cout << "ALCT hit events: " << evtsProceededA << std::endl;
}



/*****************************************************
* analysing                                          *
******************************************************/
void gifRates::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup){

	iSetup.get<MuonGeometryRecord>().get( h );
  
  double Event = iEvent.id().event();
  prevEvtN++;
  //  std::cout << prevEvtN << "\t" << Event << std::endl;
  if(int(Event)!=prevEvtN){
    std::cout << "missing evtN = " << prevEvtN << "; next event " << Event << std::endl;
    prevEvtN = int(Event);
  };
  
  //int Orbit, BX;
  //Orbit = iEvent.orbitNumber();
  //BX    = iEvent.bunchCrossing();
  if(debug)
	  std::cout << "\n\n================================================\n-=\t"<< Event << "\t=-\n";
  //std::cout << ":" << Orbit << "\t" << BX << std::endl;


  edm::Handle<CSCALCTDigiCollection> alct;
  iEvent.getByToken(alct_token, alct);

  edm::Handle<CSCWireDigiCollection> wires;
  iEvent.getByToken(wires_token, wires);
  //edm::Handle<CSCWireDigiCollection> wires;
  //iEvent.getByLabel(wireDigiTag,wires);

  handleAnodes(wires, alct);
  evtsProceededA++;

  if(!anodeOnlyTag){
	  edm::Handle<CSCRecHit2DCollection> rechits;
	  iEvent.getByLabel(cscRecHitTag, rechits);
	  handle2Dhits(rechits);
	  evtsProceeded2D++;


	  edm::Handle<CSCSegmentCollection> segments;
	  iEvent.getByLabel(cscSegTag, segments);
	  handleSegments(segments);
	  evtsProceededS++;
  };

}



/****************************************************************************************************
* code for anode hit treatment                                                                      *
* http://cmslxr.fnal.gov/source/DataFormats/CSCDigi/interface/CSCWireDigiCollection.h               *
* https://cmssdt.cern.ch/SDT/doxygen/CMSSW_7_4_15/doc/html/d3/de6/MuonDigiCollection_8h_source.html *
* TBD: distribute multiple hits over all WGs with corresponding weight                              *
* TBD: create histos with number of neighbor WGs fired                                              *                                                                                                   *
* TBD: understand how n(ALCTs) can be more than two?                                                *
*****************************************************************************************************/
void gifRates::handleAnodes(const edm::Handle<CSCWireDigiCollection> wires, const edm::Handle<CSCALCTDigiCollection> alct){

	const CSCGeometry* pgeom = &*h;

	int hitsPerLayer[6][2]; memset(hitsPerLayer, 0, sizeof(hitsPerLayer[0][0])*6*2);

	// loop over CSCWireDigiCollections
	for (CSCWireDigiCollection::DigiRangeIterator wi=wires->begin(); wi!=wires->end(); wi++) {

		CSCDetId id = (CSCDetId)(*wi).first;

		// in case of double chamber read-out
		if(id.chamber()!=chamberType) continue;

		if(debug){
			std::cout <<  "Ahits: chamber " << id.chamber()
			<< " station " << id.station()
			<< " ring    " << id.ring()
			<< " layer   " << id.layer() << std::endl;
    	};


		// just to get wire lengths
		const CSCLayer* player   =pgeom->layer(id);
		const CSCLayerGeometry * playergeometry = player->geometry();
		if(fillWL){
			for(short unsigned int i=0; i<*(nOfWGs);i++){
				wL[i] = playergeometry->lengthOfWireGroup(i+1);
				if(chamberType==1){
					wN[i] = 11;
				}else if (chamberType==2){
					if(i<64){
						wN[i] = 5;
					}else{
						wN[i] = 6;
					}
				};
			};
			if(chamberType==1){
				wN[0]  = 37;
				wN[47] = 43;
			}else if (chamberType==2){
				wN[43] = 2;
				wN[44] = 2;
				wN[79] = 3;
				wN[80] = 3;
				wN[111]= 8;
			};
			fillWL = false;
		}

		//inside the collection:
		//https://cmssdt.cern.ch/SDT/doxygen/CMSSW_7_5_1/doc/html/de/d45/CSCWireDigi_8h_source.html
		std::vector<CSCWireDigi>::const_iterator wireIt   = (*wi).second.first;
		std::vector<CSCWireDigi>::const_iterator lastWire = (*wi).second.second;

		while(wireIt != lastWire){
			int nWGh    = 0;    // number of consecutive WGs found
			bool nextWG = true; // check for next consecutive WG

			int currentWGN = wireIt->getWireGroup();

			// looking for consecutive hits
			std::vector<CSCWireDigi>::const_iterator nwgit = wireIt+1;
			while(nextWG){
				nWGh++;

				int WGN = wireIt->getWireGroup();

				if(id.layer()==1){ // just for printout
					wH1[WGN-1]++;
				};

				hAhitL[id.layer()-1][0]->Fill(WGN); // histo with all hits
				hitsPerLayer[id.layer()-1][0]++;    // hits per layer ()

				// check timing
				std::vector<int> TBs = wireIt->getTimeBinsOn();
				if(debug){
					std::cout << "\t WG: " << WGN
							<< "; length " << playergeometry->lengthOfWireGroup(WGN)
							<< "; TB: "    << wireIt->getTimeBin()
							<< " (";
					for(std::vector<int>::iterator tb=TBs.begin(); tb!=TBs.end(); ++tb){
						std::cout << (*tb) << " ";
					};std::cout << ")" << std::endl;
				};

				if ( nwgit!=lastWire ){
					nextWG = ( (nwgit->getWireGroup() - WGN) == 1 );
					++nwgit;
				}else{
					nextWG = false;
				};

				++wireIt;
			}; // end checking consecutive hits

			if(debug){
				std::cout << "current WG: " << currentWGN
						  << "; number of hits: " << nWGh
						  << std::endl;
			};
			hAhitL[id.layer()-1][1]->Fill(currentWGN); // histo with all hits
			hitsPerLayer[id.layer()-1][1]++;           // hits per layer ()
			h2dNofAhitWG[id.layer()-1]->Fill(currentWGN, nWGh);

		}; // end loop wireIt

	}; // end loop wi


	unsigned int nhits[3] = {0,0,0};
	for(short unsigned int i=0; i<6; i++){
		for(unsigned int j=0; j<2; j++){
			hNofAhitL[i][j]->Fill(hitsPerLayer[i][j]);
			nhits[j]+=hitsPerLayer[i][j];
		};
		if(hitsPerLayer[i][1]>0) {
			hNofAhitL[i][2]->Fill(1);
			nhits[2]++;
		};
	};

	for(unsigned int j=0; j<3; j++)
		hNofAhit[j]->Fill(nhits[j]);


	///////////////////////////////////////////////////////////////////////////////////////
	/*
	std::cout << "ALCT handle " << alct << std::endl;
	for (CSCALCTDigiCollection::DigiRangeIterator j=alct->begin(); j!=alct->end(); j++) {
		CSCDetId id = (CSCDetId)(*j).first;

		std::cout << "chamber " << id.chamber() << std::endl;
		std::cout << "WG of CSCALCTDigi.first = " << (*(*j).second.first).getKeyWG() << "  " << (*(*j).second.first).isValid()<< std::endl;
		std::cout << "WG of CSCALCTDigi.second = " << (*(*j).second.second).getKeyWG()<< "  " << (*(*j).second.second).isValid() << std::endl;
		std::vector<CSCALCTDigi>::const_iterator digiItr = (*j).second.first;
		std::vector<CSCALCTDigi>::const_iterator last = (*j).second.second;

		while (digiItr <= last) {
			std::cout << digiItr->getKeyWG()  << "  " << digiItr->isValid() << std::endl;
			 ++digiItr;
		};
	};
	*/
};



//===========================================================================
//===========================================================================
void gifRates::handle2Dhits  (const edm::Handle<CSCRecHit2DCollection> rechithandle){
//===========================================================================
	//int hitsPerLayer[6] = {0,0,0,0,0,0};
	for(CSCRecHit2DCollection::const_iterator rechit=rechithandle->begin(); rechit != rechithandle->end(); ++rechit){
	    CSCDetId id = (*rechit).cscDetId();

	    if(debug){
	      std::cout <<  "2Dhits: chamber " << id.chamber()
			<< " station " << id.station()
			<< " ring    " << id.ring()
			<< " layer   " << id.layer() << std::endl;
	    };

	    /*    	if(debug){
    		std::cout << "\t Wire local: " << (*rechit)->localPosition().x()
    						<< "; central wire: " << (*rechit).hitWire()
							<< "; TB: "    << wireIt->getTimeBin()
							<< " (";
    		for(std::vector<int>::iterator tb=TBs.begin(); tb!=TBs.end(); ++tb){
    			std::cout << (*tb) << " ";
    		};std::cout << ")" << std::endl;

		}*/
	};
};

//===========================================================================
//===========================================================================
int gifRates::handleSegments(const edm::Handle<CSCSegmentCollection> segments){
//===========================================================================
  //https://cmssdt.cern.ch/SDT/doxygen/CMSSW_7_5_1/doc/html/de/d34/classCSCSegment.html
  //https://cmssdt.cern.ch/SDT/doxygen/CMSSW_7_5_1/doc/html/d2/dd8/CSCSegment_8h_source.html
  // segment uses 
  int nSegments = 0;
  int nSegments4 = 0;
  if(debug)
	  std::cout << "number of Segments: " << nSegments << std::endl;

  for(CSCSegmentCollection::const_iterator dSiter=segments->begin(); dSiter != segments->end(); ++dSiter) {
    const CSCDetId sid = (*dSiter).cscDetId();
    if(sid.chamber()!=chamberType)
      continue;
    nSegments++;
    //    LocalPoint localPosition() const { return theOrigin; }
    //    LocalVector localDirection() const { return theLocalDirection; }
    //    LocalPoint theOrigin;          // in chamber frame - the GeomDet local coordinate system
    //    LocalVector theLocalDirection; // in chamber frame - the GeomDet local coordinate system
    
    const std::vector<CSCRecHit2D> seg2DhitVec = (*dSiter).specificRecHits();

    LocalPoint localPos = (*dSiter).localPosition();
    float segX     = localPos.x();
    float segY     = localPos.y();
    h2dSeg->Fill(segY, -segX);
    hsegChi2->Fill((*dSiter).chi2());
    hsegNof2Dhits->Fill( (*dSiter).nRecHits() );
    if(debug){
      std::cout << "local coordinates: " << segX << " ; " << segY << std::endl;
    };
    if(seg2DhitVec.size()>3){
        h2dSeg4->Fill(segY, -segX);
        hsegChi24->Fill((*dSiter).chi2());
        hsegNof2Dhits4->Fill( (*dSiter).nRecHits() );
        nSegments4++;
    }
    

    for(std::vector<CSCRecHit2D>::const_iterator seg2Dhit=seg2DhitVec.begin(); seg2Dhit!=seg2DhitVec.end(); ++seg2Dhit){

      //https://cmssdt.cern.ch/SDT/doxygen/CMSSW_7_5_1/doc/html/dc/d18/cscRecHitD__cfi_8py_source.html
      //https://cmssdt.cern.ch/SDT/doxygen/CMSSW_7_5_1/doc/html/df/d89/CSCRecHitDProducer_8cc_source.html      
      //      edm::ESHandle<CSCGeometry> h;
      //      setup.get<MuonGeometryRecord>().get( h );
      //      const CSCGeometry* pgeom = &*h;
      //      recHitBuilder_->setGeometry( pgeom );
      //      //https://cmssdt.cern.ch/SDT/doxygen/CMSSW_7_5_1/doc/html/d1/da0/classCSCGeometry.html
      //https://cmssdt.cern.ch/SDT/doxygen/CMSSW_7_5_1/doc/html/d6/dc1/CSCRecHitDBuilder_8h_source.html
      //https://cmssdt.cern.ch/SDT/doxygen/CMSSW_7_5_1/doc/html/d8/dcd/CSCMake2DRecHit_8cc_source.html
      // finally the localposition is calculated here:
      //        LocalPoint lp11 = layergeom_->stripWireIntersection( centerStrip, centerWire);
      //   float xWithinChamber = lp11.x();
      //                    lp0 = LocalPoint( xWithinChamber, layergeom_->yOfWire(centerWire, xWithinChamber) );
      // where const CSCLayerGeometry* layergeom_;
      //??? https://cmssdt.cern.ch/SDT/doxygen/CMSSW_7_5_1/doc/html/d7/df7/classCSCRecoConditions.html
      
      // http://cmslxr.fnal.gov/source/Geometry/CSCGeometryBuilder/data/cscSpecs.xml?v=CMSSW_7_5_1
      
      //https://cmssdt.cern.ch/SDT/doxygen/CMSSW_7_5_1/doc/html/d7/ded/CSCRecHit2D_8h_source.html
      
      const CSCDetId id = (*seg2Dhit).cscDetId();
      const CSCGeometry* pgeom = &*h;
      
      const CSCLayer* player   =pgeom->layer(id);
      const CSCLayerGeometry * playergeometry = player->geometry ();

      if(debug){
    	  std::cout <<  "chamber " << id.chamber()
		  << " station " << id.station()
		  << " layer   " << id.layer()
		  << " WG      " << (*seg2Dhit).hitWire()
		  << " recalculatedWG " << playergeometry->wireGroup(playergeometry->nearestWire(localPos))
		  << std::endl;
      };
      
    }; // end 2D hit loop
    
  }; // end segment collection loop
  hNofSeg->Fill(nSegments);
  hNofSeg4->Fill(nSegments4);
  return nSegments;
};




// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void gifRates::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(gifRates);
