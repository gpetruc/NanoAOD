import FWCore.ParameterSet.Config as cms
from PhysicsTools.NanoAOD.common_cff import *

finalMuons = cms.EDFilter("PATMuonRefSelector",
    src = cms.InputTag("slimmedMuons"),
    cut = cms.string("pt > 3 && track.isNonnull && isLooseMuon")
)

muonTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = cms.InputTag("linkedObjects","muons"),
    cut = cms.string(""), #we should not filter on cross linked collections
    name = cms.string("Muon"),
    doc  = cms.string("slimmedMuons after basic selection (" + finalMuons.cut.value()+")"),
    singleton = cms.bool(False), # the number of entries is variable
    extension = cms.bool(False), # this is the main table for the muons
    variables = cms.PSet(CandVars,
        dxy   = Var("dB", float, doc = "dxy wrt first PV, in cm"),
        ptErr   = Var("bestTrack().ptError()", float, doc = "ptError of the muon track", precision=6),
        segmentComp   = Var("segmentCompatibility()", float, doc = "ptError of the muon track", precision=6),
        nStations = Var("numberOfMatchedStations", int, doc = "number of matched stations with default arbitration (segment & track)"),
        jet = Var("?hasUserCand('jet')?userCand('jet').key():-1", int, doc="index of the associated jet (-1 if none)"),
        mediumId = Var("isPFMuon && innerTrack.validFraction >= 0.49 && ( isGlobalMuon && globalTrack.normalizedChi2 < 3 && combinedQuality.chi2LocalPosition < 12 && combinedQuality.trkKink < 20 && segmentCompatibility >= 0.303 || segmentCompatibility >= 0.451 )", bool, doc = "POG Medium muon ID (2016 tune)"),
    ),
)

muonsMCMatchForTable = cms.EDProducer("MCMatcher",       # cut on deltaR, deltaPt/Pt; pick best by deltaR
    src         = muonTable.src,                         # final reco collection
    matched     = cms.InputTag("finalGenParticles"),     # final mc-truth particle collection
    mcPdgId     = cms.vint32(13),               # one or more PDG ID (13 = mu); absolute values (see below)
    checkCharge = cms.bool(False),              # True = require RECO and MC objects to have the same charge
    mcStatus    = cms.vint32(1),                # PYTHIA status code (1 = stable, 2 = shower, 3 = hard scattering)
    maxDeltaR   = cms.double(0.3),              # Minimum deltaR for the match
    maxDPtRel   = cms.double(0.5),              # Minimum deltaPt/Pt for the match
    resolveAmbiguities    = cms.bool(True),     # Forbid two RECO objects to match to the same GEN object
    resolveByMatchQuality = cms.bool(True),    # False = just match input in order; True = pick lowest deltaR pair first
)

muonMCTable = cms.EDProducer("CandMCMatchTableProducer",
    src     = muonTable.src,
    mcMap   = cms.InputTag("muonsMCMatchForTable"),
    objName = muonTable.name,
    objType = muonTable.name, #cms.string("Muon"),
    branchName = cms.string("mcMatch"),
    docString = cms.string("MC matching to status==1 muons"),
)

muonSequence = cms.Sequence(finalMuons)
muonMC = cms.Sequence(muonsMCMatchForTable + muonMCTable)
muonTables = cms.Sequence ( muonTable)

