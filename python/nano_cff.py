import FWCore.ParameterSet.Config as cms

P4Floats = cms.PSet( pt = cms.string("pt"), eta = cms.string("eta"), phi = cms.string("phi"), mass = cms.string("mass") )
CandInts   = cms.PSet( pdgId = cms.string("pdgId"), charge = cms.string("charge"),  )

finalMuons = cms.EDFilter("PATMuonRefSelector",
    src = cms.InputTag("slimmedMuons"),
    cut = cms.string("pt > 5 && track.isNonnull && (isGlobalMuon || isTrackerMuon) && isPFMuon")
)

finalJets = cms.EDFilter("PATJetRefSelector",
    src = cms.InputTag("slimmedJets"),
    cut = cms.string("pt > 15")
)

linkedObjects = cms.EDProducer("PATObjectCrossLinker",
   jets=cms.InputTag("finalJets"),
   muons=cms.InputTag("finalMuons"),
   electrons=cms.InputTag("slimmedElectrons"),
   taus=cms.InputTag("slimmedTaus"),
   photons=cms.InputTag("slimmedPhotons"),
)

jetTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = cms.InputTag("linkedObjects","jets"),
    cut = cms.string("1"), #we should not filter on cross linked collections
    name= cms.string("Jet"),
    floats = cms.PSet(P4Floats, area = cms.string("jetArea()")),
    ints = cms.PSet(nMuonsInJet = cms.string("?hasOverlaps('muons')?overlaps('muons').size():0")),
)

muonTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = cms.InputTag("linkedObjects","muons"),
    cut = cms.string("1"), #we should not filter on cross linked collections
    name= cms.string("Muon"),
    floats = cms.PSet(P4Floats, dxy = cms.string("dB"), jetPt = cms.string("?hasUserCand('jet')?userCand('jet').pt():-1")),
    ints = cms.PSet(CandInts, nStations = cms.string("numberOfMatchedStations"), jet = cms.string("?hasUserCand('jet')?userCand('jet').key():-1")),
    bools = cms.PSet(mediumId = cms.string("isPFMuon && innerTrack.validFraction >= 0.49 && ( isGlobalMuon && globalTrack.normalizedChi2 < 3 && combinedQuality.chi2LocalPosition < 12 && combinedQuality.trkKink < 20 && segmentCompatibility >= 0.303 || segmentCompatibility >= 0.451 )")),
)

tables = cms.Sequence(finalMuons + finalJets + linkedObjects + jetTable + muonTable)

