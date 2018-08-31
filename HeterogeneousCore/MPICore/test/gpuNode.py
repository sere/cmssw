import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing

options = VarParsing('analysis')
options.register('runOnGPU',
		 False,
		 VarParsing.multiplicity.singleton,
		 VarParsing.varType.bool,
		 'If true the vector sum will be executed on the GPU')
options.register('streams',
                 1,
		 VarParsing.multiplicity.singleton,
		 VarParsing.varType.int,
		 'Number of streams to execute')
options.parseArguments()

process = cms.Process("TEST")

process.source = cms.Source("EmptySource")

process.MPIService = cms.Service("MPIService")

process.fetch = cms.EDProducer("FetchProducer")

process.work = cms.EDProducer("WorkProducer",
    runOnGPU = cms.bool(options.runOnGPU),
    job = cms.InputTag("fetch"),
    cpuID = cms.InputTag("fetch"),
    times = cms.InputTag("fetch")
)

process.send = cms.EDAnalyzer("SendAnalyzer",
    result = cms.InputTag("work"),
    cpuID = cms.InputTag("work"),
    mpiTag = cms.InputTag("fetch", "mpiTag"),
    times = cms.InputTag("work")
)

process.path = cms.Path( process.fetch + process.work + process.send )

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32( options.maxEvents )
)

process.options = cms.untracked.PSet(
    numberOfStreams = cms.untracked.uint32( 0 ),
    numberOfThreads = cms.untracked.uint32( options.streams )
)
