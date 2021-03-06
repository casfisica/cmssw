# 
#
# Read from xml and insert into database using
# PopCon 
#
# This is a template, generate real test using
#
# sed 's/EcalTBWeights/your-record/g' testTemplate.py > testyourrecord.py
#
# Stefano Argiro', $Id: testEcalTBWeights.py,v 1.1 2010/04/16 08:28:19 depasse Exp $
#
#

import FWCore.ParameterSet.Config as cms

process = cms.Process("TEST")

process.MessageLogger=cms.Service("MessageLogger",
                              destinations=cms.untracked.vstring("cout"),
                              cout=cms.untracked.PSet(
                              treshold=cms.untracked.string("INFO")
                              )
)

process.load("CondCore.DBCommon.CondDBCommon_cfi")
process.CondDBCommon.connect = cms.string('sqlite_file:testEcalTBWeights.db')
process.CondDBCommon.DBParameters.authenticationPath = cms.untracked.string('/afs/cern.ch/cms/DB/conddb')

process.source = cms.Source("EmptyIOVSource",
    timetype = cms.string('runnumber'),
    firstValue = cms.uint64(1),
    lastValue  = cms.uint64(2),
    interval = cms.uint64(1)
)

process.PoolDBOutputService = cms.Service("PoolDBOutputService",
    process.CondDBCommon,
    timetype = cms.untracked.string('runnumber'),
    toPut = cms.VPSet(cms.PSet(
        record = cms.string('EcalTBWeightsRcd'),
        tag = cms.string('mytest')
         )),
    logconnect= cms.untracked.string('sqlite_file:logtestEcalTBWeights.db')                                     
)

process.mytest = cms.EDAnalyzer("EcalTBWeightsAnalyzer",
    record = cms.string('EcalTBWeightsRcd'),
    loggingOn= cms.untracked.bool(True),
    SinceAppendMode=cms.bool(True),
    Source=cms.PSet(
    xmlFile = cms.untracked.string('/tmp/EcalTBWeights.xml'),
    since = cms.untracked.int64(3)
    )                            
)

process.p = cms.Path(process.mytest)




