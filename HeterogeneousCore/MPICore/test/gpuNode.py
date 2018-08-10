import FWCore.ParameterSet.Config as cms

process = cms.Process("TEST")

process.source = cms.Source("EmptySource")

process.fetch = cms.EDProducer("FetchProducer")

process.work = cms.EDProducer("WorkProducer",
    job = cms.InputTag("fetch"),
    times = cms.InputTag("fetch")
)

process.send = cms.EDAnalyzer("SendAnalyzer",
    result = cms.InputTag("work"),
    times = cms.InputTag("work")
)

process.path = cms.Path( process.fetch + process.work + process.send )

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32( 1001 )
)

