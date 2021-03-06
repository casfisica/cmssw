// -*- C++ -*-
//
// Package:    Phase2OuterTracker
// Class:      Phase2OuterTracker
//
/**\class Phase2OuterTracker OuterTrackerMonitorTrack.cc DQM/Phase2OuterTracker/plugins/OuterTrackerMonitorTrack.cc
 
 Description: [one line class summary]
 
 Implementation:
 [Notes on implementation]
 */
//
// Original Author:  Isis Marina Van Parijs
//         Created:  Mon, 16 Feb 2014 15:49:32 GMT
//

// system include files
#include <memory>
#include <vector>
#include <numeric>
#include <iostream>
#include <fstream>
#include <math.h>
#include "TMath.h"
#include "TNamed.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "DataFormats/Common/interface/DetSetVector.h"
#include "DataFormats/Common/interface/DetSetVectorNew.h"
#include "DQM/SiStripCommon/interface/SiStripFolderOrganizer.h"
#include "DQM/Phase2OuterTracker/interface/OuterTrackerMonitorTrack.h"
#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/MonitorElement.h"
#include "DataFormats/L1TrackTrigger/interface/TTTypes.h"
#include "DataFormats/L1TrackTrigger/interface/TTTrack.h"
#include "Geometry/TrackerGeometryBuilder/interface/PixelGeomDetUnit.h"
#include "Geometry/TrackerGeometryBuilder/interface/StackedTrackerGeometry.h"
#include "Geometry/Records/interface/StackedTrackerGeometryRecord.h"
#include "Geometry/CommonTopologies/interface/PixelTopology.h"



//
// constructors and destructor
//
OuterTrackerMonitorTrack::OuterTrackerMonitorTrack(const edm::ParameterSet& iConfig)
: dqmStore_(edm::Service<DQMStore>().operator->()), conf_(iConfig)
{
  topFolderName_ = conf_.getParameter<std::string>("TopFolderName");
  tagTTTracks_ = conf_.getParameter< edm::InputTag >("TTTracks");
  HQDelim_ = conf_.getParameter<int>("HQDelim");
}

OuterTrackerMonitorTrack::~OuterTrackerMonitorTrack()
{
  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)
}

//
// member functions
//

// ------------ method called for each event  ------------
void OuterTrackerMonitorTrack::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  /// Track Trigger Tracks
  edm::Handle< std::vector< TTTrack< Ref_PixelDigi_ > > > PixelDigiTTTrackHandle;
  iEvent.getByLabel( tagTTTracks_, PixelDigiTTTrackHandle );
  
  unsigned int numHQTracks = 0;
  unsigned int numLQTracks = 0;
  unsigned int numTracks = 0; 
  
  // Go on only if there are TTTracks from PixelDigis
  if ( PixelDigiTTTrackHandle->size() > 0 )
  {
    /// Loop over TTTracks
    unsigned int tkCnt = 0;
    std::vector< TTTrack< Ref_PixelDigi_ > >::const_iterator iterTTTrack;
    for ( iterTTTrack = PixelDigiTTTrackHandle->begin();iterTTTrack != PixelDigiTTTrackHandle->end();++iterTTTrack )
    {
      /// Make the pointer
      edm::Ptr< TTTrack< Ref_PixelDigi_ > > tempTrackPtr( PixelDigiTTTrackHandle, tkCnt++ );
      numTracks++;
      
      unsigned int nStubs = tempTrackPtr->getStubRefs().size();
      
      double trackPt = tempTrackPtr->getMomentum().perp();
      double trackPhi = tempTrackPtr->getMomentum().phi();
      double trackEta = tempTrackPtr->getMomentum().eta();
      double trackVtxZ0 = tempTrackPtr->getPOCA().z();
      double trackChi2 = tempTrackPtr->getChi2();
      double trackChi2R = tempTrackPtr->getChi2Red();
      
      Track_NStubs->Fill(nStubs);
      
      if ( nStubs >= HQDelim_ )
      {
        numHQTracks++;
        
        Track_HQ_Pt->Fill( trackPt );
        Track_HQ_Eta->Fill( trackEta );
        Track_HQ_Phi->Fill( trackPhi );
        Track_HQ_VtxZ0->Fill( trackVtxZ0 );
        Track_HQ_Chi2->Fill( trackChi2 );
        Track_HQ_Chi2Red->Fill( trackChi2R );
        
        Track_HQ_Chi2_NStubs->Fill( nStubs, trackChi2 );
        Track_HQ_Chi2Red_NStubs->Fill( nStubs, trackChi2R );
      }
      else
      {
        numLQTracks++;
        
        Track_LQ_Pt->Fill( trackPt );
        Track_LQ_Eta->Fill( trackEta );
        Track_LQ_Phi->Fill( trackPhi );
        Track_LQ_VtxZ0->Fill( trackVtxZ0 );
        Track_LQ_Chi2->Fill( trackChi2 );
        Track_LQ_Chi2Red->Fill( trackChi2R );
        
        Track_LQ_Chi2_NStubs->Fill( nStubs, trackChi2 );
        Track_LQ_Chi2Red_NStubs->Fill( nStubs, trackChi2R );
      }
    } // End of loop over TTTracks
    
  } // End TTTracks from pixeldigis 

  Track_N->Fill(numTracks); 
  Track_HQ_N->Fill( numHQTracks );
  Track_LQ_N->Fill( numLQTracks );

} // end of method


// ------------ method called once each job just before starting event loop  ------------
void
OuterTrackerMonitorTrack::beginRun(const edm::Run& run, const edm::EventSetup& es)
{
  SiStripFolderOrganizer folder_organizer;
  folder_organizer.setSiStripFolderName(topFolderName_);
  folder_organizer.setSiStripFolder();	
	std::string HistoName;
  
  dqmStore_->setCurrentFolder(topFolderName_+"/Tracks/");
  
  //Nb of tracks
  edm::ParameterSet psTrack_N =  conf_.getParameter<edm::ParameterSet>("TH1_NTracks");
  HistoName = "Track_N";
  Track_N = dqmStore_->book1D(HistoName, HistoName,
      psTrack_N.getParameter<int32_t>("Nbinsx"),
      psTrack_N.getParameter<double>("xmin"),
      psTrack_N.getParameter<double>("xmax"));
  Track_N->setAxisTitle("# L1 Tracks", 1);
  Track_N->setAxisTitle("# Events", 2);
  
  //Nb of stubs
  edm::ParameterSet psTrack_NStubs =  conf_.getParameter<edm::ParameterSet>("TH1_NStubs");
  HistoName = "Track_NStubs";
  Track_NStubs = dqmStore_->book1D(HistoName, HistoName,
      psTrack_NStubs.getParameter<int32_t>("Nbinsx"),
      psTrack_NStubs.getParameter<double>("xmin"),
      psTrack_NStubs.getParameter<double>("xmax"));
  Track_NStubs->setAxisTitle("# L1 Stubs per L1 Track", 1);
  Track_NStubs->setAxisTitle("# L1 Tracks", 2);
  
  
  /// Low-quality tracks
  dqmStore_->setCurrentFolder(topFolderName_+"/Tracks/LQ");
  
  // Nb of L1Tracks
  HistoName = "Track_LQ_N";
  Track_LQ_N = dqmStore_->book1D(HistoName, HistoName,
      psTrack_N.getParameter<int32_t>("Nbinsx"),
      psTrack_N.getParameter<double>("xmin"),
      psTrack_N.getParameter<double>("xmax"));
  Track_LQ_N->setAxisTitle("# L1 Tracks", 1);
  Track_LQ_N->setAxisTitle("# Events", 2);
  
  //Pt of the tracks
  edm::ParameterSet psTrack_Pt =  conf_.getParameter<edm::ParameterSet>("TH1_Track_Pt");
  HistoName = "Track_LQ_Pt";
  Track_LQ_Pt = dqmStore_->book1D(HistoName, HistoName,
      psTrack_Pt.getParameter<int32_t>("Nbinsx"),
      psTrack_Pt.getParameter<double>("xmin"),
      psTrack_Pt.getParameter<double>("xmax"));
  Track_LQ_Pt->setAxisTitle("p_{T} [GeV]", 1);
  Track_LQ_Pt->setAxisTitle("# L1 Tracks", 2);
  
  //Phi
  edm::ParameterSet psTrack_Phi =  conf_.getParameter<edm::ParameterSet>("TH1_Track_Phi");
  HistoName = "Track_LQ_Phi";
  Track_LQ_Phi = dqmStore_->book1D(HistoName, HistoName,
      psTrack_Phi.getParameter<int32_t>("Nbinsx"),
      psTrack_Phi.getParameter<double>("xmin"),
      psTrack_Phi.getParameter<double>("xmax"));
  Track_LQ_Phi->setAxisTitle("#phi", 1);
  Track_LQ_Phi->setAxisTitle("# L1 Tracks", 2);
  
  //Eta
  edm::ParameterSet psTrack_Eta =  conf_.getParameter<edm::ParameterSet>("TH1_Track_Eta");
  HistoName = "Track_LQ_Eta";
  Track_LQ_Eta = dqmStore_->book1D(HistoName, HistoName,
      psTrack_Eta.getParameter<int32_t>("Nbinsx"),
      psTrack_Eta.getParameter<double>("xmin"),
      psTrack_Eta.getParameter<double>("xmax"));
  Track_LQ_Eta->setAxisTitle("#eta", 1);
  Track_LQ_Eta->setAxisTitle("# L1 Tracks", 2);
  
  //VtxZ0
   edm::ParameterSet psTrack_VtxZ0 =  conf_.getParameter<edm::ParameterSet>("TH1_Track_VtxZ0");
  HistoName = "Track_LQ_VtxZ0";
  Track_LQ_VtxZ0 = dqmStore_->book1D(HistoName, HistoName,
      psTrack_VtxZ0.getParameter<int32_t>("Nbinsx"),
      psTrack_VtxZ0.getParameter<double>("xmin"),
      psTrack_VtxZ0.getParameter<double>("xmax"));
  Track_LQ_VtxZ0->setAxisTitle("L1 Track vertex position z [cm]", 1);
  Track_LQ_VtxZ0->setAxisTitle("# L1 Tracks", 2);
  
  //chi2
   edm::ParameterSet psTrack_Chi2 =  conf_.getParameter<edm::ParameterSet>("TH1_Track_Chi2");
  HistoName = "Track_LQ_Chi2";
  Track_LQ_Chi2 = dqmStore_->book1D(HistoName, HistoName,
      psTrack_Chi2.getParameter<int32_t>("Nbinsx"),
      psTrack_Chi2.getParameter<double>("xmin"),
      psTrack_Chi2.getParameter<double>("xmax"));
  Track_LQ_Chi2->setAxisTitle("L1 Track #chi^{2}", 1);
  Track_LQ_Chi2->setAxisTitle("# L1 Tracks", 2);
  
  //chi2Red
  edm::ParameterSet psTrack_Chi2Red =  conf_.getParameter<edm::ParameterSet>("TH1_Track_Chi2R");
  HistoName = "Track_LQ_Chi2Red";
  Track_LQ_Chi2Red = dqmStore_->book1D(HistoName, HistoName,
      psTrack_Chi2Red.getParameter<int32_t>("Nbinsx"),
      psTrack_Chi2Red.getParameter<double>("xmin"),
      psTrack_Chi2Red.getParameter<double>("xmax"));
  Track_LQ_Chi2Red->setAxisTitle("L1 Track #chi^{2}/ndf", 1);
  Track_LQ_Chi2Red->setAxisTitle("# L1 Tracks", 2);
  
  edm::ParameterSet psTrack_Chi2_NStubs =  conf_.getParameter<edm::ParameterSet>("TH2_Track_Chi2_NStubs");
  HistoName = "Track_LQ_Chi2_NStubs";
  Track_LQ_Chi2_NStubs = dqmStore_->book2D(HistoName, HistoName,
      psTrack_Chi2_NStubs.getParameter<int32_t>("Nbinsx"),
      psTrack_Chi2_NStubs.getParameter<double>("xmin"),
      psTrack_Chi2_NStubs.getParameter<double>("xmax"),
      psTrack_Chi2_NStubs.getParameter<int32_t>("Nbinsy"),
      psTrack_Chi2_NStubs.getParameter<double>("ymin"),
      psTrack_Chi2_NStubs.getParameter<double>("ymax"));
  Track_LQ_Chi2_NStubs->setAxisTitle("# L1 Stubs", 1);
  Track_LQ_Chi2_NStubs->setAxisTitle("L1 Track #chi^{2}", 2);
  
  edm::ParameterSet psTrack_Chi2Red_NStubs =  conf_.getParameter<edm::ParameterSet>("TH2_Track_Chi2R_NStubs");
  HistoName = "Track_LQ_Chi2Red_NStubs";
  Track_LQ_Chi2Red_NStubs = dqmStore_->book2D(HistoName, HistoName,
      psTrack_Chi2Red_NStubs.getParameter<int32_t>("Nbinsx"),
      psTrack_Chi2Red_NStubs.getParameter<double>("xmin"),
      psTrack_Chi2Red_NStubs.getParameter<double>("xmax"),
      psTrack_Chi2Red_NStubs.getParameter<int32_t>("Nbinsy"),
      psTrack_Chi2Red_NStubs.getParameter<double>("ymin"),
      psTrack_Chi2Red_NStubs.getParameter<double>("ymax"));
  Track_LQ_Chi2Red_NStubs->setAxisTitle("# L1 Stubs", 1);
  Track_LQ_Chi2Red_NStubs->setAxisTitle("L1 Track #chi^{2}/ndf", 2);
  
  
  /// High-quality tracks
  dqmStore_->setCurrentFolder(topFolderName_+"/Tracks/HQ");
  
  // Nb of L1Tracks
  HistoName = "Track_HQ_N";
  Track_HQ_N = dqmStore_->book1D(HistoName, HistoName,
      psTrack_N.getParameter<int32_t>("Nbinsx"),
      psTrack_N.getParameter<double>("xmin"),
      psTrack_N.getParameter<double>("xmax"));
  Track_HQ_N->setAxisTitle("# L1 Tracks", 1);
  Track_HQ_N->setAxisTitle("# Events", 2);
  
  //Pt of the tracks
  HistoName = "Track_HQ_Pt";
  Track_HQ_Pt = dqmStore_->book1D(HistoName, HistoName,
      psTrack_Pt.getParameter<int32_t>("Nbinsx"),
      psTrack_Pt.getParameter<double>("xmin"),
      psTrack_Pt.getParameter<double>("xmax"));
  Track_HQ_Pt->setAxisTitle("p_{T} [GeV]", 1);
  Track_HQ_Pt->setAxisTitle("# L1 Tracks", 2);
  
  //Phi
  HistoName = "Track_HQ_Phi";
  Track_HQ_Phi = dqmStore_->book1D(HistoName, HistoName,
      psTrack_Phi.getParameter<int32_t>("Nbinsx"),
      psTrack_Phi.getParameter<double>("xmin"),
      psTrack_Phi.getParameter<double>("xmax"));
  Track_HQ_Phi->setAxisTitle("#phi", 1);
  Track_HQ_Phi->setAxisTitle("# L1 Tracks", 2);
  
  //Eta
  HistoName = "Track_HQ_Eta";
  Track_HQ_Eta = dqmStore_->book1D(HistoName, HistoName,
      psTrack_Eta.getParameter<int32_t>("Nbinsx"),
      psTrack_Eta.getParameter<double>("xmin"),
      psTrack_Eta.getParameter<double>("xmax"));
  Track_HQ_Eta->setAxisTitle("#eta", 1);
  Track_HQ_Eta->setAxisTitle("# L1 Tracks", 2);
  
  //VtxZ0
  HistoName = "Track_HQ_VtxZ0";
  Track_HQ_VtxZ0 = dqmStore_->book1D(HistoName, HistoName,
      psTrack_VtxZ0.getParameter<int32_t>("Nbinsx"),
      psTrack_VtxZ0.getParameter<double>("xmin"),
      psTrack_VtxZ0.getParameter<double>("xmax"));
  Track_HQ_VtxZ0->setAxisTitle("L1 Track vertex position z [cm]", 1);
  Track_HQ_VtxZ0->setAxisTitle("# L1 Tracks", 2);
  
  //chi2
  HistoName = "Track_HQ_Chi2";
  Track_HQ_Chi2 = dqmStore_->book1D(HistoName, HistoName,
      psTrack_Chi2.getParameter<int32_t>("Nbinsx"),
      psTrack_Chi2.getParameter<double>("xmin"),
      psTrack_Chi2.getParameter<double>("xmax"));
  Track_HQ_Chi2->setAxisTitle("L1 Track #chi^{2}", 1);
  Track_HQ_Chi2->setAxisTitle("# L1 Tracks", 2);
  
  //chi2Red
  HistoName = "Track_HQ_Chi2Red";
  Track_HQ_Chi2Red = dqmStore_->book1D(HistoName, HistoName,
      psTrack_Chi2Red.getParameter<int32_t>("Nbinsx"),
      psTrack_Chi2Red.getParameter<double>("xmin"),
      psTrack_Chi2Red.getParameter<double>("xmax"));
  Track_HQ_Chi2Red->setAxisTitle("L1 Track #chi^{2}/ndf", 1);
  Track_HQ_Chi2Red->setAxisTitle("# L1 Tracks", 2);
  
  HistoName = "Track_HQ_Chi2_NStubs";
  Track_HQ_Chi2_NStubs = dqmStore_->book2D(HistoName, HistoName,
      psTrack_Chi2_NStubs.getParameter<int32_t>("Nbinsx"),
      psTrack_Chi2_NStubs.getParameter<double>("xmin"),
      psTrack_Chi2_NStubs.getParameter<double>("xmax"),
      psTrack_Chi2_NStubs.getParameter<int32_t>("Nbinsy"),
      psTrack_Chi2_NStubs.getParameter<double>("ymin"),
      psTrack_Chi2_NStubs.getParameter<double>("ymax"));
  Track_HQ_Chi2_NStubs->setAxisTitle("# L1 Stubs", 1);
  Track_HQ_Chi2_NStubs->setAxisTitle("L1 Track #chi^{2}", 2);
  
  HistoName = "Track_HQ_Chi2Red_NStubs";
  Track_HQ_Chi2Red_NStubs = dqmStore_->book2D(HistoName, HistoName,
      psTrack_Chi2Red_NStubs.getParameter<int32_t>("Nbinsx"),
      psTrack_Chi2Red_NStubs.getParameter<double>("xmin"),
      psTrack_Chi2Red_NStubs.getParameter<double>("xmax"),
      psTrack_Chi2Red_NStubs.getParameter<int32_t>("Nbinsy"),
      psTrack_Chi2Red_NStubs.getParameter<double>("ymin"),
      psTrack_Chi2Red_NStubs.getParameter<double>("ymax"));
  Track_HQ_Chi2Red_NStubs->setAxisTitle("# L1 Stubs", 1);
  Track_HQ_Chi2Red_NStubs->setAxisTitle("L1 Track #chi^{2}/ndf", 2);
  
}//end of method

// ------------ method called once each job just after ending the event loop  ------------
void 
OuterTrackerMonitorTrack::endJob(void) 
{
	
}

DEFINE_FWK_MODULE(OuterTrackerMonitorTrack);
