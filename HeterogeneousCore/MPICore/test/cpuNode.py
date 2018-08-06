import FWCore.ParameterSet.Config as cms

process = cms.Process("TEST")

process.source = cms.Source("EmptySource")

process.produce = cms.EDProducer("StringProducer",
    message = cms.string("Hello world")
)

process.offload = cms.EDProducer("OffloadProducer",
    msg_source = cms.InputTag("produce")
)

process.analyze = cms.EDAnalyzer("CoutAnalyzer",
    source = cms.InputTag("offload")
)

process.path = cms.Path( process.produce + process.offload + process.analyze )

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32( 2 )
)

