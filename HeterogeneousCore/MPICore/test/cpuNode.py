import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing

options = VarParsing('analysis')
options.register('vlen',
		 1,
		 VarParsing.multiplicity.singleton,
		 VarParsing.varType.float,
		 'Length of the vectors to be summed')
options.parseArguments()

process = cms.Process("TEST")

process.source = cms.Source("EmptySource")

process.produce = cms.EDProducer("ArraysProducer",
    vectorLength = cms.int32(int(options.vlen))
)

process.offload = cms.EDProducer("OffloadProducer",
    arrays = cms.InputTag("produce")
)

process.analyze = cms.EDAnalyzer("PrintAnalyzer",
    result = cms.InputTag("offload"),
    times = cms.InputTag("offload")
)

process.path = cms.Path( process.produce + process.offload + process.analyze )

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32( options.maxEvents )
)

