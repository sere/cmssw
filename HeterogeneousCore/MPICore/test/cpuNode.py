import FWCore.ParameterSet.Config as cms

process = cms.Process("TEST")

process.source = cms.Source("EmptySource")

process.produce = cms.EDProducer("ArraysProducer")

process.offload = cms.EDProducer("OffloadProducer",
    arrays = cms.InputTag("produce")
)

process.analyze = cms.EDAnalyzer("PrintAnalyzer",
    result = cms.InputTag("offload")
)

process.path = cms.Path( process.produce + process.offload + process.analyze )

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32( 1 )
)

