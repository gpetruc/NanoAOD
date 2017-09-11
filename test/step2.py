import FWCore.ParameterSet.Config as cms
process = cms.Process('cmsMerge')

process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.options   = cms.untracked.PSet( wantSummary = cms.untracked.bool(True) )
process.MessageLogger.cerr.FwkReport.reportEvery = 10
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(-1))

process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring())
process.source.fileNames = ['file:step1.root']

process.out = cms.OutputModule("NanoAODOutputModule",
    fileName = cms.untracked.string('nano_merged.root'),
    outputCommands = process.NanoAODEDMEventContent.outputCommands,
#    compressionLevel = cms.untracked.int(9),
#    compressionAlgorithm = cms.untracked.string("LZMA"),

)
process.end = cms.EndPath(process.out)  
