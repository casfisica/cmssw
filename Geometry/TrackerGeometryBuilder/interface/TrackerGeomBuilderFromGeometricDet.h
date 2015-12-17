#ifndef Geometry_TrackerGeometryBuilder_TrackerGeomBuilderFromGeometricDet_H
#define Geometry_TrackerGeometryBuilder_TrackerGeomBuilderFromGeometricDet_H

#include <string>
#include <vector>
#include <map>
#include "DataFormats/TrackerCommon/interface/TrackerTopology.h"
#include "Geometry/TrackerGeometryBuilder/interface/PlaneBuilderFromGeometricDet.h"
#include "Geometry/TrackerNumberingBuilder/interface/GeometricDet.h"
#include "Geometry/CommonDetUnit/interface/GeomDetType.h"

class TrackerGeometry;
class TrackerTopology;
class PixelGeomDetType;
class StripGeomDetType;

class TrackerGeomBuilderFromGeometricDet {
public:

  TrackerGeometry* build(const GeometricDet* gd, const edm::ParameterSet& pSet, const TrackerTopology* tTopo );

private:

  void buildPixel(std::vector<const GeometricDet*> const &,
		  TrackerGeometry*,GeomDetType::SubDetector det,
		  bool upgradeGeometry,
		  int BIG_PIX_PER_ROC_X,
		  int BIG_PIX_PER_ROC_Y);
  void buildSilicon(std::vector<const GeometricDet*> const &, TrackerGeometry*,
		    const TrackerTopology*, GeomDetType::SubDetector det, const std::string& part);
  void buildGeomDet(TrackerGeometry*, bool upgradeGeometry, const TrackerTopology* tTopo);

  PlaneBuilderFromGeometricDet::ResultType
  buildPlaneWithMaterial(const GeometricDet* gd,double scaleFactor=1.) const;

  std::map<std::string,PixelGeomDetType*> thePixelDetTypeMap;
  std::map<std::string,StripGeomDetType*> theStripDetTypeMap;
};

#endif
