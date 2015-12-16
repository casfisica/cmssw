// -*- C++ -*-
//
// Package:     ParticleFlow
// Class  :     FWCandidate3DProxyBuilder
// 
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Colin Bernet
//         Created:  Fri May 28 15:58:19 CEST 2010
// Edited:           sharris, Wed 9 Feb 2011, 17:34
//
#include <set>
#include "Fireworks/FWInterface/interface/ContextFF.h"
// System include files
#include "TEveTrack.h"
#include "TEveTrackPropagator.h"
#include "TRandom3.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"
#include "DataFormats/ParticleFlowReco/interface/PFBlockElement.h"
#include "DataFormats/ParticleFlowReco/interface/PFCluster.h"
#include "DataFormats/ParticleFlowReco/interface/PFBlock.h"
#include "Fireworks/Core/interface/FWProxyBuilderConfiguration.h"

// User include files
#include "Fireworks/Core/interface/FWSimpleProxyBuilderTemplate.h"
#include "Fireworks/Core/interface/FWEvePtr.h"
#include "Fireworks/Core/src/CmsShowMain.h"
#include "Fireworks/Core/interface/FWEventItem.h"
//#include "Fireworks/ParticleFlow/interface/setTrackTypePF.h"
#include "Fireworks/Core/interface/FWGeometry.h"
#include "Fireworks/Core/interface/BuilderUtils.h"
#include "TEveCompound.h"
#include "TEveBoxSet.h"
#include "TEveManager.h"
#include "TEveScene.h"

#include "FWCore/Framework/interface/ESHandle.h"

#include "Fireworks/Core/interface/FWProxyBuilderBase.h"
#include "Geometry/FCalGeometry/interface/HGCalGeometry.h"
#include "DataFormats/ForwardDetId/interface/HGCalDetId.h"
#include "Geometry/CaloGeometry/interface/CaloCellGeometry.h"
//-----------------------------------------------------------------------------
// FWPFCandidate3DProxyBuilderFF
//-----------------------------------------------------------------------------

class FWPFCandidate3DProxyBuilderFF : public FWSimpleProxyBuilderTemplate<reco::PFCandidate>
{
      
public:
   // ---------------- Constructor(s)/Destructor ----------------------
   FWPFCandidate3DProxyBuilderFF() { myRandom.SetSeed(0); }
   virtual ~FWPFCandidate3DProxyBuilderFF() {}
   
   REGISTER_PROXYBUILDER_METHODS();
   void debugGeo();

   virtual void setItem(const FWEventItem* iItem);
protected:

   virtual void localModelChanges(const FWModelId& iId, TEveElement* iCompound,
                                  FWViewType::EType viewType, const FWViewContext* vc);

private:
   TRandom3 myRandom;

   FWPFCandidate3DProxyBuilderFF( const FWPFCandidate3DProxyBuilderFF& );                    // Stop default
   const FWPFCandidate3DProxyBuilderFF& operator=( const FWPFCandidate3DProxyBuilderFF& );   // Stop default

   // --------------------- Member Functions --------------------------
   void build( const reco::PFCandidate& iData, unsigned int iIndex, TEveElement& oItemHolder, const FWViewContext* );
};

//______________________________________________________________________________


void 
FWPFCandidate3DProxyBuilderFF::setItem(const FWEventItem* iItem)
{
   FWProxyBuilderBase::setItem(iItem);
 
   iItem->getConfig()->assertParam("HGCalEESensitive", true);
   iItem->getConfig()->assertParam("HGCalHESiliconSensitive", true);
   iItem->getConfig()->assertParam("HGCalHEScintillatorSensitive", true);
}
//______________________________________________________________________________




void
FWPFCandidate3DProxyBuilderFF::localModelChanges(const FWModelId& iId, TEveElement* iCompound,
                                     FWViewType::EType viewType, const FWViewContext* vc)
{
   increaseComponentTransparency(iId.index(), iCompound, "Trap", 90);
}


//______________________________________________________________________________
void 
FWPFCandidate3DProxyBuilderFF::build( const reco::PFCandidate& iData, unsigned int iIndex, TEveElement& oItemHolder, const FWViewContext* ) 
{
   const fireworks::ContextFF* ctx = dynamic_cast<const fireworks::ContextFF*>(&item()->context()); 
   std::vector<edm::ESHandle<HGCalGeometry> >  geoHandles;
   try {
      const edm::EventSetup* setup = ctx->getEventSetup();
      assert(setup);
      geoHandles.push_back(edm::ESHandle<HGCalGeometry>());
      setup->get<IdealGeometryRecord>().get("HGCalEESensitive", geoHandles.back());
      geoHandles.push_back(edm::ESHandle<HGCalGeometry>());
      setup->get<IdealGeometryRecord>().get("HGCalHESiliconSensitive", geoHandles.back());
      geoHandles.push_back(edm::ESHandle<HGCalGeometry>());
      setup->get<IdealGeometryRecord>().get("HGCalHEScintillatorSensitive", geoHandles.back());
   }
   catch (std::exception& e) {
      std::cout << "FWPFCandidate3DProxyBuilderFF" <<  e.what() << std::endl;
   }

   bool layerEnable [] = {false, false, false};

   if ( item()->getConfig()->value<bool>("HGCalEESensitive"))
      layerEnable[0] = true;
   if ( item()->getConfig()->value<bool>("HGCalHESiliconSensitive"))
      layerEnable[1] = true;
   if ( item()->getConfig()->value<bool>("HGCalHEScintillatorSensitive"))
      layerEnable[2] = true;

  const FWDisplayProperties &dp = item()->defaultDisplayProperties();
 
  const reco::PFCandidate::ElementsInBlocks& elems = iData.elementsInBlocks();
  
  for( unsigned i = 0 ; i < elems.size(); ++i ) {
    const reco::PFBlockElement& elem = elems[i].first->elements()[elems[i].second];
    switch( elem.type() ) {
    case reco::PFBlockElement::TRACK:
      {
	TEveRecTrack t;
	t.fBeta = 1.;
	t.fP = TEveVector( iData.px(), iData.py(), iData.pz() );
	t.fV = TEveVector( iData.vertex().x(), iData.vertex().y(), iData.vertex().z() );
	t.fSign = iData.charge();
	TEveTrack* trk = new TEveTrack(&t, context().getTrackPropagator() );      
	trk->MakeTrack();      
	setupAddElement( trk, &oItemHolder, false );
      }
      break;
    case reco::PFBlockElement::ECAL:
    case reco::PFBlockElement::HCAL:
    case reco::PFBlockElement::HGC_ECAL:
    case reco::PFBlockElement::HGC_HCALF:
    case reco::PFBlockElement::HGC_HCALB:
      {
	if( elem.clusterRef().isNull() || !elem.clusterRef().isAvailable() ) {
	  TEveRecTrack t;
	  t.fBeta = 1.;
	  t.fP = TEveVector( iData.px(), iData.py(), iData.pz() );
	  t.fV = TEveVector( iData.vertex().x(), iData.vertex().y(), iData.vertex().z() );
	  t.fSign = iData.charge();
	  TEveTrack* trk = new TEveTrack(&t, context().getTrackPropagator() );      
	  trk->MakeTrack();      
	  setupAddElement( trk, &oItemHolder, true );
	  continue;
	}
	const std::vector<std::pair<DetId, float> >& clusterDetIds = 
	  elem.clusterRef()->hitsAndFractions();	
	TEveBoxSet* boxset = new TEveBoxSet(  );
	boxset->Reset(TEveBoxSet::kBT_FreeBox, true, clusterDetIds.size()  );
	boxset->UseSingleColor();

	
	TEveBoxSet* boxsetTop = new TEveBoxSet("Trap");
	boxsetTop->Reset(TEveBoxSet::kBT_FreeBox, true, clusterDetIds.size()  );
	boxsetTop->UseSingleColor();

        std::vector<float> pnts;
        pnts.resize(24);
        std::set< const CaloCellGeometry*> trapSet;
	for( std::vector<std::pair<DetId, float> >::const_iterator it = clusterDetIds.begin(), itEnd = clusterDetIds.end();
	     it != itEnd; ++it )
        {
           DetId dId = (*it).first ;
           int layerCnt =0;
           for( const auto& hgcGeom : geoHandles ){
              // skip if disabled in collection controller
              if (!layerEnable[layerCnt++]) continue;


              // cell corners
              static const float maxd = 10000.0f;
              try {
                 int c = 0;
                 const HGCalGeometry::CornersVec cor = hgcGeom->getCorners( dId );
                 for( const auto& corner : cor ) {
                    pnts[c*3] = corner.x();
                    pnts[c*3+1] = corner.y();
                    pnts[c*3+2] = corner.z();

                    if (TMath::Abs(corner.x()) > maxd || TMath::Abs(corner.y())> maxd || TMath::Abs(corner.z())> maxd ) 
                        throw std::runtime_error("Error in vertices " );
                    c++;
                 }

                 boxset->AddBox(&pnts[0]);
              }
              catch (std::exception &e) {
                 //  std::cout << "AMT get cell corners invalid ID " << e.what() << std::endl;
              }
              catch (...) {
                 std::cout << "unknown cell\n";
              }


              // trapezoid corners
              try {
                 const CaloCellGeometry* cell = hgcGeom->getGeometry( dId);
                 if (trapSet.find(cell) == trapSet.end()) {
                    trapSet.insert(cell);
                    CaloCellGeometry::CornersVec dd = cell->getCorners(); 
                    int c = 0;
                    for( std::vector<GlobalPoint>::const_iterator i = dd.begin(); i != dd.end(); ++i )
                    {  
                       if (TMath::Abs(i->x()) > maxd || TMath::Abs(i->y()) > maxd  || TMath::Abs(i->z()) > maxd  )
                          throw std::runtime_error("Error in vertices " );
              
                       pnts[c*3] = i->x();
                       pnts[c*3+1] = i->y();
                       pnts[c*3+2] = i->z();
                       c++;
                    }
                    boxsetTop->AddBox( &pnts[0]);
                 }
              }
              catch (std::exception &e) {
                 //std::cout << "AMT get trapezoid corners invalid ID " << e.what() << std::endl;
              }
              catch (...) {

                 std::cout << "unknown trap\n";
              }
           }
	  }
	boxset->RefitPlex();
	setupAddElement(boxset,&oItemHolder, true);
	setupAddElement(boxsetTop,&oItemHolder, true);
        boxsetTop->SetMainTransparency(TMath::Min(100, 90 + dp.transparency() / 5));
      }
      break;
    default:
      break;
    }
  }
}
//______________________________________________________________________________


void 
FWPFCandidate3DProxyBuilderFF::debugGeo()
{/*
TEveElement* scene =  gEve->GetScenes()->FindChild("GeoScene 3D RecHit");
if (!scene) return;
   printf("debugGeo !!!\n");
   if (geomH.isValid())
      printf("VALID !!!i\n");
   else 
      printf("INVALID \n");

  TEvePointSet* pointSet = new TEvePointSet("pnts");
      gEve->AddElement(pointSet);
      TEveBoxSet* boxSet = new TEveBoxSet("cells-CaloSubGeometry");
      boxSet->UseSingleColor();
      boxSet->SetMainColor(kBlue);
      boxSet->Reset(TEveBoxSet::kBT_FreeBox, true, 6000);
      gEve->AddElement(boxSet);


    TEveBoxSet* boxSetTop = new TEveBoxSet("cells-CaloCellGeometery");
      boxSetTop->UseSingleColor();
      boxSetTop->SetMainColor(kRed);
      boxSetTop->Reset(TEveBoxSet::kBT_FreeBox, true, 6000);
      gEve->AddElement(boxSetTop);

      float pnts[24];

      const std::vector<DetId>& hids = geomH->getValidDetIds();
      for( const auto& hid : hids ) {
         HGCalDetId hgid = HGCalDetId(hid.rawId());
         if (hgid.sector() == 8 && hgid.layer() == 103) {

            const HGCalGeometry::CornersVec cor( std::move( geomH->getCorners( hid ) ) );
            int c = 0;
            for( const auto& corner : cor ) {
               // pointSet->SetNextPoint(corner.x(), corner.y(), corner.z());
               pnts[c*3] = corner.x();
               pnts[c*3+1] = corner.y();
               pnts[c*3+2] = corner.z();
               c++;
            }
	    boxSet->AddBox(&pnts[0]);
            
            // standard Calo interface, which gives false results
            const CaloCellGeometry* cell = geomH->getGeometry( hid);
            CaloCellGeometry::CornersVec dd = cell->getCorners();
            std::vector<float> pntsx(8);
            for( std::vector<GlobalPoint>::const_iterator i = dd.begin(); i != dd.end(); ++i )
            {
               pntsx.push_back(i->x());
               pntsx.push_back(i->y());
               pntsx.push_back(i->z());
            }

            boxSetTop->AddBox(&pntsx[0]);
            
            //std::cout << hgid << std::endl;
         }
      }
      boxSet->RefitPlex();
      boxSetTop->RefitPlex();

      scene->AddElement(boxSet);
      scene->AddElement(boxSetTop);
 */
}

//______________________________________________________________________________
REGISTER_FWPROXYBUILDER(FWPFCandidate3DProxyBuilderFF, reco::PFCandidate,"PFCandidatesFF", FWViewType::kAll3DBits | FWViewType::kAllRPZBits );
