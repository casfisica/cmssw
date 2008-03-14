#include "EventFilter/CSCRawToDigi/interface/CSCDDUDataItr.h"
#include "EventFilter/CSCRawToDigi/interface/CSCDDUEventData.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"


// theCurrentCSC starts at -1, since user is expected to next() before he dereferences
// for the first time
CSCDDUDataItr::CSCDDUDataItr() :
  theDDUData(0),
  theCurrentCSC(-1),
  theNumberOfCSCs(0),
  theDataIsOwnedByMe(true)
{}


CSCDDUDataItr::CSCDDUDataItr(const char * buf) :
  theDDUData(0),
  theCurrentCSC(-1),
  theNumberOfCSCs(0),
  theDataIsOwnedByMe(true)
{
  // check if it's OK first
  const CSCDDUHeader * dduHeader
    = reinterpret_cast<const CSCDDUHeader *>(buf);
  if(dduHeader->check()){
    theDDUData = new CSCDDUEventData((unsigned short *)buf);
    theNumberOfCSCs = theDDUData->cscData().size();
  } else {
    edm::LogError ("CSCDDUDataItr") << "OMG! FAILED THE HEADER CHECK ";
  }
}
  

CSCDDUDataItr::CSCDDUDataItr(const CSCDDUEventData * dduData) :
  theDDUData(dduData),
  theCurrentCSC(-1),
  theNumberOfCSCs(theDDUData->cscData().size()),
  theDataIsOwnedByMe(false)
{
}


CSCDDUDataItr::~CSCDDUDataItr() 
{
  if(theDataIsOwnedByMe) delete theDDUData;
}


CSCDDUDataItr::CSCDDUDataItr(const CSCDDUDataItr & i) :
  theCurrentCSC(i.theCurrentCSC),
  theNumberOfCSCs(i.theNumberOfCSCs),
  theDataIsOwnedByMe(i.theDataIsOwnedByMe)
{
  if(theDataIsOwnedByMe) 
    {
      if(i.theDDUData != 0) 
	{
	  theDDUData = new CSCDDUEventData(*(i.theDDUData));
	}
    }
  else
    {
      theDDUData = i.theDDUData;
    }
}


void CSCDDUDataItr::operator=(const CSCDDUDataItr & i) 
{
  if(theDataIsOwnedByMe) 
    {
      delete theDDUData;
      if(i.theDDUData != 0) 
	{
	  theDDUData = new CSCDDUEventData(*(i.theDDUData));
	}
    }
  else
    {
      theDDUData = i.theDDUData;
    }

  theDDUData = i.theDDUData;
  theCurrentCSC = i.theCurrentCSC;
  theNumberOfCSCs = i.theNumberOfCSCs;
  theDataIsOwnedByMe = i.theDataIsOwnedByMe;
}  


bool CSCDDUDataItr::next() 
{
  return (++theCurrentCSC < theNumberOfCSCs);
}


const CSCEventData &  CSCDDUDataItr::operator*() 
{
  assert(theCurrentCSC >= 0 && theCurrentCSC < theNumberOfCSCs);
  return theDDUData->cscData()[theCurrentCSC];
}

 
