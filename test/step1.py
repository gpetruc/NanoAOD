import FWCore.ParameterSet.Config as cms
process = cms.Process('NANO')

process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.options   = cms.untracked.PSet( wantSummary = cms.untracked.bool(True) )
process.MessageLogger.cerr.FwkReport.reportEvery = 10
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(-1))

process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring())
process.source.fileNames = [
#relvals:
# '/store/relval/CMSSW_9_3_0_pre4/RelValTTbar_13/MINIAODSIM/93X_mc2017_realistic_v1-v1/00000/1CFF7C9C-6A86-E711-A1F2-0CC47A7C35F4.root',
# '/store/relval/CMSSW_9_3_0_pre4/RelValTTbar_13/MINIAODSIM/93X_mc2017_realistic_v1-v1/00000/107D499F-6A86-E711-8A51-0025905B8592.root',
        
#sample with LHE
        '/store/mc/RunIISummer17MiniAOD/TT_TuneCUETP8M2T4_13TeV-powheg-pythia8/MINIAODSIM/92X_upgrade2017_realistic_v10_ext1-v1/110000/187F7EDA-0986-E711-ABB3-02163E014C21.root'
]
process.load("PhysicsTools.NanoAOD.nano_cff")

process.nanoPath = cms.Path(process.nanoSequenceMC)
#for data:
#process.nanoPath = cms.Path(process.nanoSequence)

process.out = cms.OutputModule("PoolOutputModule",
   fileName = cms.untracked.string("step1.root"),
   outputCommands = cms.untracked.vstring("drop *", "keep *Table_*Table_*_*","keep edmTriggerResults_*_*_*"),
)

process.end = cms.EndPath(process.out)  
