import FWCore.ParameterSet.Config as cms
process = cms.Process('NANO')

process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.options   = cms.untracked.PSet( wantSummary = cms.untracked.bool(True) )
process.MessageLogger.cerr.FwkReport.reportEvery = 100
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(-1))

process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring())
process.source.fileNames = [
#relvals:
# '/store/relval/CMSSW_9_3_0_pre4/RelValTTbar_13/MINIAODSIM/93X_mc2017_realistic_v1-v1/00000/1CFF7C9C-6A86-E711-A1F2-0CC47A7C35F4.root',
# '/store/relval/CMSSW_9_3_0_pre4/RelValTTbar_13/MINIAODSIM/93X_mc2017_realistic_v1-v1/00000/107D499F-6A86-E711-8A51-0025905B8592.root',

"/store/relval/CMSSW_9_3_0_pre4/JetHT/MINIAOD/93X_dataRun2_relval_v0_RelVal_rmd_jetHT2016E-v1/00000/84AA0159-838C-E711-8CAF-0CC47A4C8F0A.root",
"/store/relval/CMSSW_9_3_0_pre4/JetHT/MINIAOD/93X_dataRun2_relval_v0_RelVal_rmd_jetHT2016E-v1/00000/8A482659-838C-E711-A0B9-0CC47A4C8ED8.root",
"/store/relval/CMSSW_9_3_0_pre4/JetHT/MINIAOD/93X_dataRun2_relval_v0_RelVal_rmd_jetHT2016E-v1/00000/EC590840-798C-E711-B6AB-0025905A60A0.root"
]

process.load("PhysicsTools.NanoAOD.nano_cff")


#rocess.nanoPath = cms.Path(process.nanoSequenceMC)
#for data:
process.nanoPath = cms.Path(process.nanoSequence)

process.out = cms.OutputModule("NanoAODOutputModule",
    fileName = cms.untracked.string('nano.root'),
    outputCommands = process.NanoAODEDMEventContent.outputCommands,
   #compressionLevel = cms.untracked.int32(9),
    #compressionAlgorithm = cms.untracked.string("LZMA"),

)
process.out1 = cms.OutputModule("NanoAODOutputModule",
    fileName = cms.untracked.string('lzma.root'),
    outputCommands = process.NanoAODEDMEventContent.outputCommands,
    compressionLevel = cms.untracked.int32(9),
    compressionAlgorithm = cms.untracked.string("LZMA"),

)
process.end = cms.EndPath(process.out+process.out1)  
