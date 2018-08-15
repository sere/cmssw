import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing

options = VarParsing('analysis')
options.register('runOnGPU',
		 False,
		 VarParsing.multiplicity.singleton,
		 VarParsing.varType.bool,
		 'If true the vector sum will be executed on the GPU')
options.parseArguments()

process = cms.Process("TEST")

process.source = cms.Source("EmptySource")

process.fetch = cms.EDProducer("FetchProducer")

process.work = cms.EDProducer("WorkProducer",
    runOnGPU = cms.bool(options.runOnGPU),
    job = cms.InputTag("fetch"),
    times = cms.InputTag("fetch")
)

process.send = cms.EDAnalyzer("SendAnalyzer",
    result = cms.InputTag("work"),
    times = cms.InputTag("work")
)

process.path = cms.Path( process.fetch + process.work + process.send )

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32( options.maxEvents )
)

