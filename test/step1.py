import FWCore.ParameterSet.Config as cms
process = cms.Process('NANO')

process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.options   = cms.untracked.PSet( wantSummary = cms.untracked.bool(True) )
process.MessageLogger.cerr.FwkReport.reportEvery = 10
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(100))

process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring())
process.source.fileNames = [
	'/store/relval/CMSSW_9_3_0_pre4/RelValTTbar_13/MINIAODSIM/93X_mc2017_realistic_v1-v1/00000/1CFF7C9C-6A86-E711-A1F2-0CC47A7C35F4.root',
]
process.load("PhysicsTools.NanoAOD.nano_cff")


process.tablesPath = cms.Path(process.tables)

process.out = cms.OutputModule("PoolOutputModule",
   fileName = cms.untracked.string("step1.root"),
   outputCommands = cms.untracked.vstring("drop *", "keep *_*Table_*_*"),
)

process.end = cms.EndPath(process.out)  
