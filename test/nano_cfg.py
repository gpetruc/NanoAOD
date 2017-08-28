import FWCore.ParameterSet.Config as cms
process = cms.Process('cmsMerge')

process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.options   = cms.untracked.PSet( wantSummary = cms.untracked.bool(True) )
process.MessageLogger.cerr.FwkReport.reportEvery = 10
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(10000))

process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring())
process.source.fileNames = [
	'/store/relval/CMSSW_9_3_0_pre4/RelValTTbar_13/MINIAODSIM/93X_mc2017_realistic_v1-v1/00000/1CFF7C9C-6A86-E711-A1F2-0CC47A7C35F4.root',
]


P4Floats = cms.PSet( pt = cms.string("pt"), eta = cms.string("eta"), phi = cms.string("phi"), mass = cms.string("mass") )
CandInts   = cms.PSet( pdgId = cms.string("pdgId"), charge = cms.string("charge"),  )


process.linkedObjects = cms.EDProducer("PATObjectCrossLinker",
   jets=cms.InputTag("slimmedJets"),
   muons=cms.InputTag("slimmedMuons"),
)

process.jetTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = cms.InputTag("linkedObjects","jets"),
    cut = cms.string("pt > 15"),
    name= cms.string("Jet"),
    floats = cms.PSet(P4Floats, area = cms.string("jetArea()")),
)
process.muonTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = cms.InputTag("linkedObjects","muons"),
    cut = cms.string("pt > 5 && track.isNonnull && (isGlobalMuon || isTrackerMuon) && isPFMuon"),
    name= cms.string("Muon"),
    floats = cms.PSet(P4Floats, dxy = cms.string("dB"), jetPt = cms.string("?hasUserCand('jet')?userCand('jet').pt():-1")),
    ints = cms.PSet(CandInts, nStations = cms.string("numberOfMatchedStations"), jet = cms.string("?hasUserCand('jet')?userCand('jet').key():-1")),
    bools = cms.PSet(mediumId = cms.string("isPFMuon && innerTrack.validFraction >= 0.49 && ( isGlobalMuon && globalTrack.normalizedChi2 < 3 && combinedQuality.chi2LocalPosition < 12 && combinedQuality.trkKink < 20 && segmentCompatibility >= 0.303 || segmentCompatibility >= 0.451 )")),
)

process.tables = cms.Path(process.linkedObjects + process.jetTable + process.muonTable)

process.out = cms.OutputModule("NanoAODOutputModule",
    fileName = cms.untracked.string('nano.root'),
    outputCommands = cms.untracked.vstring("drop *", "keep *_*Table_*_*"),

)
process.end = cms.EndPath(process.out)  
