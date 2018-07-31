import FWCore.ParameterSet.Config as cms

process = cms.Process("TEST")

process.source = cms.Source("EmptySource")

process.produce = cms.EDProducer("StringProducer",
    message = cms.string("Hello world")
)

process.analyze = cms.EDAnalyzer("MPIAnalyzer",
    msg_source = cms.InputTag("produce")
)

process.path = cms.Path( process.produce + process.analyze )

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32( 1 )
)

