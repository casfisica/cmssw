#ifndef GEMDigi_GEMCSCPadDigiCollection_h
#define GEMDigi_GEMCSCPadDigiCollection_h
/** \class GEMCSCPadDigiCollection
 *  
 *  \author Vadim Khotilovich
 */

#include <DataFormats/MuonDetId/interface/GEMDetId.h>
#include <DataFormats/GEMDigi/interface/GEMCSCPadDigi.h>
#include <DataFormats/MuonData/interface/MuonDigiCollection.h>

typedef MuonDigiCollection<GEMDetId, GEMCSCPadDigi> GEMCSCPadDigiCollection;

#endif

