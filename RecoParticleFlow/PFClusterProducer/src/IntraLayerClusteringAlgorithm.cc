/*
 *
 * IntraLayerClusteringAlgorithm.cc source template automatically generated by a class generator
 * Creation date : sam. f�vr. 8 2014
 *
 * This file is part of ArborPFA libraries.
 * 
 * ArborPFA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * based upon these libraries are permitted. Any copy of these libraries
 * must include this copyright notice.
 * 
 * ArborPFA is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with ArborPFA.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * @author remi
 * @author L. Gray, FNAL (for port to CMSSW) 
 * @copyright CNRS , IPNL
 */

// note: we have ripped this still-beating heart out of the pandora framework
//       into a considerably more sane place
/*****~~~~~ Kali Ma... Kali Ma... ~~~~~*****/

#include "RecoParticleFlow/PFClusterProducer/interface/InitialClusteringStepBase.h"
#include "DataFormats/ParticleFlowReco/interface/PFRecHitFraction.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include <unordered_map>

#ifdef PFLOW_DEBUG
#define LOGVERB(x) edm::LogVerbatim(x)
#define LOGWARN(x) edm::LogWarning(x)
#define LOGERR(x) edm::LogError(x)
#define LOGDRESSED(x) edm::LogInfo(x)
#else
#define LOGVERB(x) LogTrace(x)
#define LOGWARN(x) edm::LogWarning(x)
#define LOGERR(x) edm::LogError(x)
#define LOGDRESSED(x) LogDebug(x)
#endif

#include "DataFormats/ForwardDetId/interface/HGCEEDetId.h"
#include "DataFormats/ForwardDetId/interface/HGCHEDetId.h"
namespace {
  template<typename DETID>
  std::pair<int,int> getlayer(const unsigned rawid) {
    DETID id(rawid);
    return std::make_pair(id.zside(),id.layer());
  }
}

class IntraLayerClusteringAlgorithm : public InitialClusteringStepBase {
  typedef IntraLayerClusteringAlgorithm B2DGT;
 public:
  typedef std::unordered_multimap<int,unsigned> layer_map;
  IntraLayerClusteringAlgorithm(const edm::ParameterSet& conf) :
    InitialClusteringStepBase(conf),
    m_intraLayerMaxDistance(std::pow(0.1*conf.getParameter<double>("IntraLayerMaxDistance"),2.0)), // mm to cm and save the square
    m_maximumSizeForClusterSplitting(conf.getParameter<unsigned>("MaximumSizeForClusterSplitting")),
    m_shouldSplitClusterInSingleCaloHitClusters(conf.getParameter<bool>("ShouldSplitClusterInSingleCaloHitClusters")){ }
  virtual ~IntraLayerClusteringAlgorithm() {}
  IntraLayerClusteringAlgorithm(const B2DGT&) = delete;
  B2DGT& operator=(const B2DGT&) = delete;

  void buildClusters(const edm::Handle<reco::PFRecHitCollection>&,
		     const std::vector<bool>&,
		     const std::vector<bool>&, 
		     reco::PFClusterCollection&) override;
  
 private:  
  
  void RecursiveClustering(const edm::Handle<reco::PFRecHitCollection>& input,
			   const std::vector<bool>& hitsInLayers,
			   const reco::PFRecHitRef& cell,
			   std::vector<bool>& used,		 
			   reco::PFCluster& topocluster) const;

  void SplitClusterInSingleCaloHitClusters(const reco::PFCluster& temp,
					   reco::PFClusterCollection& output) const;

  const double m_intraLayerMaxDistance;
  const unsigned m_maximumSizeForClusterSplitting;
  const bool m_shouldSplitClusterInSingleCaloHitClusters;  
};

DEFINE_EDM_PLUGIN(InitialClusteringStepFactory,
		  IntraLayerClusteringAlgorithm,
		  "IntraLayerClusteringAlgorithm");

void IntraLayerClusteringAlgorithm::
buildClusters(const edm::Handle<reco::PFRecHitCollection>& input,
	      const std::vector<bool>& rechitMask,
	      const std::vector<bool>& seedable, 
	      reco::PFClusterCollection& output) {
  const reco::PFRecHitCollection& inp = *input;
  std::vector<bool> used(input->size(),false);
  // layer -> hit index in two endcaps
  layer_map hitsByLayer[2]; 
  // group hits by layer (assuming one detector here)
  std::pair<int,int> hgc_layer; 
  PFLayer::Layer the_detector = PFLayer::NONE;
  for( unsigned i = 0; i < inp.size(); ++i ) {
    const reco::PFRecHit& hit = inp[i];
    the_detector = hit.layer();
    // skip non-HGCAL detectors for now
    if( !rechitMask[i] || the_detector < 13 ) continue; 
    // determine our layer
    if( the_detector == 13 ) {
      hgc_layer = std::move(getlayer<HGCEEDetId>(hit.detId()));
    } else {
      hgc_layer = std::move(getlayer<HGCHEDetId>(hit.detId()));
    }
    hitsByLayer[(hgc_layer.first+1)/2].emplace(hgc_layer.second,i);         
  }

  reco::PFCluster temp;
  for( const auto& side : hitsByLayer ) {
    // loop over the layers (32 = magic number from detid)
    for( int layer = 0; layer < 32; ++layer ) { 
      const auto hits = side.equal_range(layer);
      for( auto ihit = hits.first; ihit != hits.second; ++ihit ) {
	const unsigned hit = ihit->second;
	if( used[hit] ) continue;
	temp.reset();
	RecursiveClustering(input,rechitMask,makeRefhit(input,hit),used,temp);
	if( m_shouldSplitClusterInSingleCaloHitClusters && 
	    temp.recHitFractions().size() > m_maximumSizeForClusterSplitting ){
	  SplitClusterInSingleCaloHitClusters(temp,output);
	} else {
	  double x(0.0),y(0.0),z(0.0);
	  for( const auto& rhf : temp.recHitFractions() ) {
	    const math::XYZPoint& pos = rhf.recHitRef()->position();
	    x += pos.x();
	    y += pos.y();
	    z += pos.z();
	  }
	  double sizeinv = 1.0/temp.recHitFractions().size();
	  x *= sizeinv; y *= sizeinv; z *= sizeinv;
	  const math::XYZPoint mypos(x,y,z);
	  temp.setLayer(the_detector);
	  temp.setPosition(mypos);
	  temp.calculatePositionREP();
	  temp.setEnergy(temp.recHitFractions().size());
	  output.push_back(temp);
	}
      }
    }
  }
  edm::LogError("ClusterReport") 
    << "made " << output.size() << " topological clusters!";
}

/*****~~~~~ Kali Ma... Kali Ma... ~~~~~*****/
void IntraLayerClusteringAlgorithm::
RecursiveClustering(const edm::Handle<reco::PFRecHitCollection>& input,
		    const std::vector<bool>& rechitMask,
		    const reco::PFRecHitRef& cell,
		    std::vector<bool>& used,		 
		    reco::PFCluster& topocluster) const {
  if( used[cell.key()] ) return;
  const reco::PFRecHitCollection& inp = *input;
  used[cell.key()] = true;
  topocluster.addRecHitFraction(reco::PFRecHitFraction(cell, 1.0));  
  for( unsigned hit = 0; hit < inp.size(); ++hit ) {
    if( hit == cell.key() || used[hit] || !rechitMask[hit] ) continue;
    const math::XYZPoint cellp = cell->position();
    const math::XYZPoint hitp  = inp[hit].position();
    const double d2 = (hitp - cellp).Mag2();
    if( d2 <= m_intraLayerMaxDistance ) {
      RecursiveClustering(input,rechitMask,
			  makeRefhit(input,hit),
			  used,topocluster);
    }
  }
}

/*****~~~~~ Kali Ma Shakti de!!! ~~~~~*****/
void IntraLayerClusteringAlgorithm::
SplitClusterInSingleCaloHitClusters(const reco::PFCluster& temp,
				    reco::PFClusterCollection& output) const {
  for( const auto& rhf : temp.recHitFractions() ) {
    const math::XYZPoint& pos = rhf.recHitRef()->position();
    output.push_back(reco::PFCluster(rhf.recHitRef()->layer(),1.0,
				     pos.x(),pos.y(),pos.z()) );
    output.back().addRecHitFraction(rhf);
  }
}
