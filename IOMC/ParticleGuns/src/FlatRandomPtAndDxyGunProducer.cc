#include <ostream>

#include "IOMC/ParticleGuns/interface/FlatRandomPtAndDxyGunProducer.h"

#include "SimDataFormats/GeneratorProducts/interface/HepMCProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Utilities/interface/RandomNumberGenerator.h"
#include "DataFormats/Math/interface/deltaR.h"

#include "CLHEP/Random/RandFlat.h"

using namespace edm;
using namespace std;

FlatRandomPtAndDxyGunProducer::FlatRandomPtAndDxyGunProducer(const ParameterSet& pset) :
BaseFlatGunProducer(pset)
{
  ParameterSet defpset ;
  ParameterSet pgun_params =
    pset.getParameter<ParameterSet>("PGunParameters") ;

  fMinPt = pgun_params.getParameter<double>("MinPt");
  fMaxPt = pgun_params.getParameter<double>("MaxPt");
  dxyMin_ = pgun_params.getParameter<double>("dxyMin");
  dxyMax_ = pgun_params.getParameter<double>("dxyMax");
  LzMin_ = pgun_params.getParameter<double>("LzMin");
  LzMax_ = pgun_params.getParameter<double>("LzMax");

  produces<HepMCProduct>();
  produces<GenEventInfoProduct>();
}

FlatRandomPtAndDxyGunProducer::~FlatRandomPtAndDxyGunProducer()
{
    // no need to cleanup GenEvent memory - done in HepMCProduct
}

void FlatRandomPtAndDxyGunProducer::produce(Event &e, const EventSetup& es)
{
  if ( fVerbosity > 0 ){
    cout << " FlatRandomPtAndDxyGunProducer : Begin New Event Generation" << endl ;
  }
  // event loop (well, another step in it...)

  // no need to clean up GenEvent memory - done in HepMCProduct
  //

  // here re-create fEvt (memory)
  //
  fEvt = new HepMC::GenEvent() ;

  // now actualy, cook up the event from PDGTable and gun parameters
  int barcode = 1 ;
  for (unsigned int ip=0; ip<fPartIDs.size(); ++ip){

    double phi_vtx = 0;
    double dxy = 0;
    double pt = 0;
    double eta = 0;
    double px = 0;
    double py = 0;
    double pz = 0;
    double vx = 0;
    double vy = 0;
    double vz = 0;
    double lxy = 0;

    bool passLxy = false;
    while (not passLxy) {

      phi_vtx = fRandomGenerator->fire(fMinPhi, fMaxPhi);
      dxy = fRandomGenerator->fire(dxyMin_, dxyMax_);
      pt = fRandomGenerator->fire(fMinPt, fMaxPt);
      px = pt*cos(phi_vtx);
      py = pt*sin(phi_vtx);
      for (int i=0; i<10000; i++){
        vx = fRandomGenerator->fire(-dxyMax_, dxyMax_);
        vy = (pt*dxy + vx * py)/px;
        lxy = sqrt(vx*vx + vy*vy);
        if (lxy<abs(dxyMax_)){
          passLxy = true;
          break;
        }
      }
      eta = fRandomGenerator->fire(fMinEta, fMaxEta);
      pz = pt * sinh(eta);
      vz = fRandomGenerator->fire(-LzMin_, LzMax_);

      if (passLxy)
        break;
    }

    HepMC::GenVertex* Vtx1  = new HepMC::GenVertex(HepMC::FourVector(vx,vy,vz));

    int PartID = fPartIDs[ip] ;
    const HepPDT::ParticleData* PData = fPDGTable->particle(HepPDT::ParticleID(abs(PartID))) ;
    double mass   = PData->mass().value() ;
    double energy2= px*px + py*py + pz*pz + mass*mass ;
    double energy = sqrt(energy2) ;
    HepMC::FourVector p(px,py,pz,energy) ;
    HepMC::GenParticle* Part = new HepMC::GenParticle(p,PartID,1);
    Part->suggest_barcode( barcode ) ;
    barcode++;
    Vtx1->add_particle_out(Part);
    fEvt->add_vertex(Vtx1) ;

    if ( fAddAntiParticle ) {
      HepMC::GenVertex* Vtx2  = new HepMC::GenVertex(HepMC::FourVector(-vx,-vy,-vz));
      HepMC::FourVector ap(-px,-py,-pz,energy) ;
      int APartID = -PartID ;
      if ( PartID == 22 || PartID == 23 )
        {
          APartID = PartID ;
        }
      HepMC::GenParticle* APart = new HepMC::GenParticle(ap,APartID,1);
      APart->suggest_barcode( barcode ) ;
      barcode++ ;
      Vtx2->add_particle_out(APart) ;
      fEvt->add_vertex(Vtx2) ;
    }
  }
  fEvt->set_event_number(e.id().event()) ;
  fEvt->set_signal_process_id(20) ;

  if ( fVerbosity > 0 ){
    fEvt->print() ;
  }

  auto_ptr<HepMCProduct> BProduct(new HepMCProduct()) ;
  BProduct->addHepMCData( fEvt );
  e.put(BProduct);

  auto_ptr<GenEventInfoProduct> genEventInfo(new GenEventInfoProduct(fEvt));
  e.put(genEventInfo);

  if ( fVerbosity > 0 ){
    cout << " FlatRandomPtAndDxyGunProducer : End New Event Generation" << endl ;
    fEvt->print();
  }
}
