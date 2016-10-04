#include "Validation/GEMCR/interface/gemcrValidation.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"

#include "SimDataFormats/TrackingHit/interface/PSimHitContainer.h"
#include "Geometry/GEMGeometry/interface/GEMSuperChamber.h"

#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/GeometrySurface/interface/Bounds.h"
#include "DataFormats/GeometryVector/interface/LocalPoint.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "DataFormats/TrajectorySeed/interface/PropagationDirection.h"
#include "DataFormats/Math/interface/Vector.h"
#include "DataFormats/Math/interface/Point3D.h"
#include "DataFormats/Common/interface/RefToBase.h" 
#include "DataFormats/TrajectoryState/interface/PTrajectoryStateOnDet.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateTransform.h"

#include "DataFormats/HepMCCandidate/interface/GenParticle.h"




#include <iomanip>


using namespace std;

gemcrValidation::gemcrValidation(const edm::ParameterSet& cfg): GEMBaseValidation(cfg)
{
  InputTagToken_   = consumes<edm::PSimHitContainer>(cfg.getParameter<edm::InputTag>("simInputLabel"));
  InputTagToken_RH = consumes<GEMRecHitCollection>(cfg.getParameter<edm::InputTag>("recHitsInputLabel"));
  InputTagToken_TR = consumes<vector<reco::Track>>(cfg.getParameter<edm::InputTag>("tracksInputLabel"));
  InputTagToken_TS = consumes<vector<TrajectorySeed>>(cfg.getParameter<edm::InputTag>("seedInputLabel"));
  InputTagToken_GP = consumes<vector<reco::GenParticle>>(cfg.getParameter<edm::InputTag>("genParticleLabel"));
  edm::ParameterSet serviceParameters = cfg.getParameter<edm::ParameterSet>("ServiceParameters");
  theService = new MuonServiceProxy(serviceParameters);
}

void gemcrValidation::bookHistograms(DQMStore::IBooker & ibooker, edm::Run const & Run, edm::EventSetup const & iSetup ) {
  GEMGeometry_ = initGeometry(iSetup);
  if ( GEMGeometry_ == nullptr) return ;  

  const std::vector<const GEMSuperChamber*>& superChambers_ = GEMGeometry_->superChambers();   
  for (auto sch : superChambers_){
    int n_lay = sch->nChambers();
    for (int l=0;l<n_lay;l++){
      gemChambers.push_back(*sch->chamber(l+1));
    }
  }
  n_ch = gemChambers.size();

  ibooker.setCurrentFolder("MuonGEMRecHitsV/GEMRecHitsTask");

  gemcr_g = ibooker.book3D("gemcr_g","GEMCR GLOBAL RECHITS", 500,-25,25,20,-55,55, 140,0,140);
  gem_cls_tot = ibooker.book1D("cluster_size","Cluseter size",20,0,20);  
  gem_bx_tot = ibooker.book1D("bx", "BX" , 30, -15,15);
  tr_size = ibooker.book1D("tr_size", "track size",10,0,10);
  tr_hit_size = ibooker.book1D("tr_hit_size", "hit size in track",15,0,15); 
  del_rx = ibooker.book1D("rec_tr_del_x", "rec-tr del x", 200,-10,10);
  del_ry = ibooker.book1D("rec_tr_del_y", "rec-tr del y", 200,-100,100);

  tr_eff = ibooker.book1D("tr_eff", "track in track/chamber",n_ch,0,n_ch);
  hit_eff = ibooker.book1D("hit_eff", "tracking hit in track/chamber",n_ch,0,n_ch);
  tr_chamber = ibooker.book1D("tr_eff_ch", "tr /chamber",n_ch,0,n_ch); 
  th_chamber = ibooker.book1D("th_eff_ch", "tr hit/chamber",n_ch,0,n_ch); 
  rh_chamber = ibooker.book1D("rh_eff_ch", "rec hit/chamber",n_ch,0,n_ch); 
  sh_chamber = ibooker.book1D("sh_eff_ch", "sim hit/chamber",n_ch,0,n_ch); 
  for(int c = 0; c<n_ch; c++){
   cout << gemChambers[c].id() << endl;
   GEMDetId gid = gemChambers[c].id();
   string b_name = "chamber_"+to_string(gid.chamber())+"_layer_"+to_string(gid.layer());
   tr_eff->setBinLabel(c+1,b_name);
   hit_eff->setBinLabel(c+1,b_name);
   tr_chamber->setBinLabel(c+1,b_name);
   th_chamber->setBinLabel(c+1,b_name);
   rh_chamber->setBinLabel(c+1,b_name);
   sh_chamber->setBinLabel(c+1,b_name);
  }
  for(int c = 0; c<n_ch;c++){
     GEMDetId gid = gemChambers[c].id();
     string h_name = "chamber_"+to_string(gid.chamber())+"_layer_"+to_string(gid.layer());
     gem_chamber_x_y.push_back(ibooker.book2D(h_name+"_x_roll",h_name+" x vs roll", 500,-25,25,10,0,10));
     gem_chamber_cl_size.push_back(ibooker.book2D(h_name+"_cl_size", h_name+" cluster size", 20,0,20,10,0,10));
     gem_chamber_firedStrip.push_back(ibooker.book2D(h_name+"_firedStrip", h_name+" fired strip", 385,0,385,10,0,10));
     gem_chamber_bx.push_back(ibooker.book2D(h_name+"_bx", h_name+" BX", 30,-15,15,10,0,10));
     gem_chamber_tr_eff.push_back(ibooker.book1D(h_name+"_tr_eff", h_name+" tr eff", 10,0,10));
     gem_chamber_ch_eff.push_back(ibooker.book1D(h_name+"_ch_eff", h_name+" ch eff", 10,0,10));
     gem_chamber_rec_eff.push_back(ibooker.book1D(h_name+"_rec_eff", h_name+" rec eff", 10,0,10));
  }
}

int gemcrValidation::findIndex(GEMDetId id_){
  int index=-1;
  for(int c =0;c<n_ch;c++){
    if((gemChambers[c].id().chamber() == id_.chamber())&(gemChambers[c].id().layer() == id_.layer()) ){index = c;}
  }
  return index;
}

const GEMGeometry* gemcrValidation::initGeometry(edm::EventSetup const & iSetup) {
  const GEMGeometry* GEMGeometry_ = nullptr;
  try {
    edm::ESHandle<GEMGeometry> hGeom;
    iSetup.get<MuonGeometryRecord>().get(hGeom);
    GEMGeometry_ = &*hGeom;
  }
  catch( edm::eventsetup::NoProxyException<GEMGeometry>& e) {
    edm::LogError("MuonGEMBaseValidation") << "+++ Error : GEM geometry is unavailable on event loop. +++\n";
    return nullptr;
  }
  return GEMGeometry_;
}

gemcrValidation::~gemcrValidation() {
}

void gemcrValidation::analyze(const edm::Event& e, const edm::EventSetup& iSetup){

  theService->update(iSetup);

  edm::Handle<GEMRecHitCollection> gemRecHits;
  edm::Handle<edm::PSimHitContainer> gemSimHits;
  edm::Handle<vector<reco::Track>> gemTracks;
  edm::Handle<vector<TrajectorySeed>> trSeed;
  edm::Handle<vector<reco::GenParticle>> gMuon; 

  e.getByToken( this->InputTagToken_, gemSimHits);
  e.getByToken( this->InputTagToken_RH, gemRecHits);
  e.getByToken( this->InputTagToken_TR, gemTracks);
  e.getByToken( this->InputTagToken_TS, trSeed);
  e.getByToken( this->InputTagToken_GP, gMuon);
  if (!gemRecHits.isValid()) {
    edm::LogError("gemcrValidation") << "Cannot get strips by Token RecHits Token.\n";
    return ;
  }
  vector<bool> checkRH, checkSH, checkTH, checkTR;
  vector<double> tr_hit_x, tr_hit_y, rec_hit_x, rec_hit_y;
  for(int c=0;c<n_ch;c++){
    checkRH.push_back(0);
    checkSH.push_back(0);
    checkTH.push_back(0);
    checkTR.push_back(0);
    tr_hit_x.push_back(-99);
    tr_hit_y.push_back(-99);
    rec_hit_x.push_back(-99);
    rec_hit_y.push_back(-99);
  }
  for (edm::PSimHitContainer::const_iterator hits = gemSimHits->begin(); hits!=gemSimHits->end(); ++hits) {
    const GEMDetId id(hits->detUnitId());
    int index = findIndex(id);
    checkSH[index] = 1;    
  }
  for (GEMRecHitCollection::const_iterator recHit = gemRecHits->begin(); recHit != gemRecHits->end(); ++recHit){

    Float_t  rh_l_x = recHit->localPosition().x();
    Int_t  bx = recHit->BunchX();
    Int_t  clusterSize = recHit->clusterSize();
    Int_t  firstClusterStrip = recHit->firstClusterStrip();

    GEMDetId id((*recHit).gemId());
    int index = findIndex(id);
    checkRH[index] = 1;
    Short_t rh_roll = (Short_t) id.roll();
    LocalPoint recHitLP = recHit->localPosition();
    if ( GEMGeometry_->idToDet((*recHit).gemId()) == nullptr) {
      std::cout<<"This gem recHit did not matched with GEMGeometry."<<std::endl;
      continue;
    }

    GlobalPoint recHitGP = GEMGeometry_->idToDet((*recHit).gemId())->surface().toGlobal(recHitLP);

    Float_t     rh_g_X = recHitGP.x();
    Float_t     rh_g_Y = recHitGP.y();
    Float_t     rh_g_Z = recHitGP.z();
    gem_chamber_x_y[index]->Fill(rh_l_x, rh_roll);
    gem_chamber_cl_size[index]->Fill(clusterSize,rh_roll);
    gem_chamber_bx[index]->Fill(bx,rh_roll);
    gemcr_g->Fill(rh_g_X,rh_g_Z,rh_g_Y);
    gem_cls_tot->Fill(clusterSize);
    gem_bx_tot->Fill(bx);
     
    std::vector<int> stripsFired;
    for(int i = firstClusterStrip; i < (firstClusterStrip + clusterSize); i++){
      gem_chamber_firedStrip[index]->Fill(i,rh_roll);
      stripsFired.push_back(i);
    }

    rec_hit_x[index] = rh_g_X;
    rec_hit_y[index] = rh_g_Z;
    
  }

  if (gemTracks.isValid()){
    tr_size->Fill(gemTracks.product()->size());
    
    for(auto tr: *gemTracks.product()){
      TrajectorySeed seed = trSeed.product()->at(0);
      PTrajectoryStateOnDet ptsd1(seed.startingState());
      DetId did(ptsd1.detId());
      const BoundPlane& bp = theService->trackingGeometry()->idToDet(did)->surface();
      TrajectoryStateOnSurface tsos = trajectoryStateTransform::transientState(ptsd1,&bp,&*theService->magneticField());
      TrajectoryStateOnSurface tsosCurrent = tsos;

      vector<bool> check_cham, check_tr;
      vector<int> check_tr_roll, check_ch_roll;
      for(int c=0; c<n_ch;c++){
        check_cham.push_back(0);
        check_tr.push_back(0);
        check_tr_roll.push_back(-1);
        check_ch_roll.push_back(-1);
     }
      for(trackingRecHit_iterator hit = tr.recHitsBegin(); hit != tr.recHitsEnd(); hit++){
        //LocalPoint tr_rh_lp = (*hit)->localPosition();
        GEMDetId tid = GEMDetId((*hit)->geographicalId());
        int index = findIndex(tid);
        check_cham[index] = 1;
        checkTH[index] = 1;
        check_ch_roll[index] = tid.roll();
        check_tr_roll[index] = tid.roll();
        //rec_hit_x[index] = tr_rh_lp.x();
        //rec_hit_y[index] = tr_rh_lp.y();
      }
      for(int c=0; c<n_ch;c++){
        GEMChamber ch = gemChambers[c];
        tsosCurrent = theService->propagator("SteppingHelixPropagatorAny")->propagate(tsosCurrent,ch.surface());
        Global3DPoint gtrp = tsosCurrent.freeTrajectoryState()->position();
        Local3DPoint tlp = ch.surface().toLocal(gtrp);
        tr_hit_x[c] = gtrp.x();
        tr_hit_y[c] = gtrp.z();
        //if (ch.surface().bounds().inside(tlp)){checkTH[c]=1;}
          int n_roll = ch.nEtaPartitions();
          bool mat_eta = 0;
          for (int r=0;r<n_roll;r++){
            int n_strip = ch.etaPartition(r+1)->nstrips();
            Local3DPoint rtlp = ch.etaPartition(r+1)->surface().toLocal(gtrp);
            double min_x = ch.etaPartition(r+1)->centreOfStrip(0).x();
            double max_x = ch.etaPartition(r+1)->centreOfStrip(n_strip).x();
            if ((abs(rtlp.y()) < 0.1) & (rtlp.x()>min_x) & (rtlp.x() < max_x) &(abs(tr_hit_x[c]-rec_hit_x[c])<5)){ check_tr_roll[c] = r+1; mat_eta = 1;checkTR[c] = 1;}
          }
          if (!mat_eta) {cout << "no mat eta chamber : " << c << " tr x  : " << tlp.x() << ", tr y : " << tlp.y()<<  endl; cout << "rechit roll : " << check_ch_roll[c] << endl;}

          if (check_cham[c] & !check_tr_roll[c]) {check_tr_roll[c] = check_ch_roll[c];} 
        //}
        if (check_tr_roll[c] >0){   
          check_tr[c]=1; 
        }
      }
      bool checkEv = 1;
      for(int c=0;c<n_ch;c++){
         if (checkTH[c] && !checkTR[c]){checkEv = 0;}
         //if (rec_hit_x[c] != -99 && tr_hit_x[c] == -99){checkEv = 0;}
         //if (abs(rec_hit_x[c]-tr_hit_x[c]) > 5){checkEv = 0;}
      }
      if (!checkEv){continue;}
      for(int c=0;c<n_ch;c++){
        if (checkSH[c]){sh_chamber->Fill(c);}
        if (checkRH[c]){rh_chamber->Fill(c);}
        if (checkTH[c]){th_chamber->Fill(c);}
        if (checkTR[c]){tr_chamber->Fill(c);}
        del_ry->Fill(rec_hit_y[c]-tr_hit_y[c]);
        del_rx->Fill(rec_hit_x[c]-tr_hit_x[c]);
        //if (check_rec[c]){rec_chamber->Fill(c);}
        //if (check_cham[c]) {hit_eff->Fill(c); gem_chamber_ch_eff[c]->Fill(check_tr_roll[c]);}
        //if (check_tr[c]){tr_eff->Fill(c); gem_chamber_tr_eff[c]->Fill(check_tr_roll[c]);}
      }

    }
  }
}


