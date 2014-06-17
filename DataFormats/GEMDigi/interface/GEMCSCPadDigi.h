#ifndef GEMDigi_GEMCSCPadDigi_h
#define GEMDigi_GEMCSCPadDigi_h

/** \class GEMCSCPadDigi
 *
 * Digi for GEM-CSC trigger pads
 *  
 * \author Vadim Khotilovich
 *
 */

#include <boost/cstdint.hpp>
#include <iosfwd>

class GEMCSCPadDigi{

public:
  explicit GEMCSCPadDigi (int pad, int bx, int roll);
  GEMCSCPadDigi ();

  bool operator==(const GEMCSCPadDigi& digi) const;
  bool operator!=(const GEMCSCPadDigi& digi) const;
  bool operator<(const GEMCSCPadDigi& digi) const;

  int pad() const { return pad_; }
  int bx() const { return bx_; }
  int roll() const { return roll_; }

  void print() const;

private:
  uint16_t pad_;
  int32_t  bx_;
  uint8_t  roll_;
};

std::ostream & operator<<(std::ostream & o, const GEMCSCPadDigi& digi);

#endif

