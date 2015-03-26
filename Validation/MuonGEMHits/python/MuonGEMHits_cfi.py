import FWCore.ParameterSet.Config as cms


gemHitsValidation = cms.EDAnalyzer('GEMHitsValidation',
    verboseSimHit = cms.untracked.int32(1),
    simInputLabel = cms.InputTag('g4SimHits',"MuonGEMHits"),
    # st1, st2_short, st2_long of xbin, st1,st2_short,st2_long of ybin
    nBinGlobalZR = cms.untracked.vdouble(200,200,200,110,140,210), 
    # st1 xmin, xmax, st2_short xmin, xmax, st2_long xmin, xmax, st1 ymin, ymax...
    RangeGlobalZR = cms.untracked.vdouble(564,572,786,794,794,802,130,240,190,330,120,330),
    nBinGlobalXY = cms.untracked.int32(720), 
)

gemSimTrackValidation = cms.EDAnalyzer('GEMSimTrackMatch',
    verboseSimHit = cms.untracked.int32(1),
    simInputLabel = cms.InputTag('g4SimHits',"MuonGEMHits"),
    simMuOnlyGEM = cms.bool(True),
    discardEleHitsGEM = cms.bool(True),
    simTrackCollection = cms.InputTag('g4SimHits'),
    simVertexCollection = cms.InputTag('g4SimHits'),
)

gemSimValid = cms.Sequence( gemHitsValidation+gemSimTrackValidation)
