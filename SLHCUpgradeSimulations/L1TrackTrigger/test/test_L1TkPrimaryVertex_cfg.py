import FWCore.ParameterSet.Config as cms

process = cms.Process("VTX")

#
# This runs over a file containing L1Tracks, and determines the
# event primary vertex by running the L1TkFastVertexProducer.
#


process.load("FWCore.MessageService.MessageLogger_cfi")

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(100) )


process.source = cms.Source("PoolSource",
   fileNames = cms.untracked.vstring(
    #'/store/group/comm_trigger/L1TrackTrigger/620_SLHC10/Extended2023TTI/Neutrinos/PU140_newGT/m1_SingleNeutrino_E2023TTI_PU140.root'
    #
    # SingleElectron test file :
    '/store/group/comm_trigger/L1TrackTrigger/620_SLHC10/Extended2023TTI/Electrons/PU140/m1_SingleElectron_E2023TTI_PU140.root'
   )
)


# ---- Global Tag :
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'PH2_1K_FB_V3::All', '')


# --- Produce the L1TkPrimaryVertex

process.load('Geometry.TrackerGeometryBuilder.StackedTrackerGeometry_cfi')
#process.load('Configuration.StandardSequences.MagneticField_38T_PostLS1_cff')
process.load('Configuration.Geometry.GeometryExtended2023TTIReco_cff')


process.load("SLHCUpgradeSimulations.L1TrackTrigger.L1TkPrimaryVertexProducer_cfi")
process.pL1TkPrimaryVertex = cms.Path( process.L1TkPrimaryVertex )




process.Out = cms.OutputModule( "PoolOutputModule",
    fileName = cms.untracked.string( "example.root" ),
    fastCloning = cms.untracked.bool( False ),
    outputCommands = cms.untracked.vstring( 'drop *')
)


# --- Keep the object containing the L1 primary vertex :
process.Out.outputCommands.append('keep *_L1TkPrimaryVertex_*_*')

# --- Keep the genParticles to allow a trivial access to the generated vertex in the ROOT file
process.Out.outputCommands.append('keep *_genParticles_*_*')
#process.Out.outputCommands.append('keep *_generator_*_*')

# --- to use the genParticles, one needs to keep the collections of associators below:
process.Out.outputCommands.append('keep *_TTTrackAssociatorFromPixelDigis_*_*')
process.Out.outputCommands.append('keep *_TTStubAssociatorFromPixelDigis_*_*')
process.Out.outputCommands.append('keep *_TTClusterAssociatorFromPixelDigis_*_*')


process.FEVToutput_step = cms.EndPath(process.Out)




