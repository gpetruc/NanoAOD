import FWCore.ParameterSet.Config as cms
from PhysicsTools.NanoAOD.common_cff import *

finalPhotons = cms.EDFilter("PATPhotonRefSelector",
    src = cms.InputTag("slimmedPhotons"),
    cut = cms.string("pt > 5 ")
)

photonTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = cms.InputTag("linkedObjects","photons"),
    cut = cms.string(""), #we should not filter on cross linked collections
    name= cms.string("Photon"),
    doc = cms.string("slimmedPhotons after basic selection (" + finalPhotons.cut.value()+")"),
    singleton = cms.bool(False), # the number of entries is variable
    extension = cms.bool(False), # this is the main table for the photons
    variables = cms.PSet(CandVars,
       jet = Var("?hasUserCand('jet')?userCand('jet').key():-1", int, doc="index of the associated jet (-1 if none)"),
    )
)

photonsMCMatchForTable = cms.EDProducer("MCMatcher",  # cut on deltaR, deltaPt/Pt; pick best by deltaR
    src         = photonTable.src,                 # final reco collection
    matched     = cms.InputTag("finalGenParticles"), # final mc-truth particle collection
    mcPdgId     = cms.vint32(11,22),                 # one or more PDG ID (11 = el, 22 = pho); absolute values (see below)
    checkCharge = cms.bool(False),              # True = require RECO and MC objects to have the same charge
    mcStatus    = cms.vint32(1),                # PYTHIA status code (1 = stable, 2 = shower, 3 = hard scattering)
    maxDeltaR   = cms.double(0.3),              # Minimum deltaR for the match
    maxDPtRel   = cms.double(0.5),              # Minimum deltaPt/Pt for the match
    resolveAmbiguities    = cms.bool(True),     # Forbid two RECO objects to match to the same GEN object
    resolveByMatchQuality = cms.bool(True),    # False = just match input in order; True = pick lowest deltaR pair first
)

photonMCTable = cms.EDProducer("CandMCMatchTableProducer",
    src     = photonTable.src,
    mcMap   = cms.InputTag("photonsMCMatchForTable"),
    objName = photonTable.name,
    objType = photonTable.name, #cms.string("Photon"),
    branchName = cms.string("mcMatch"),
    docString = cms.string("MC matching to status==1 photons or electrons"),
)

photonSequence = cms.Sequence(finalPhotons)
photonTables = cms.Sequence ( photonTable)
photonMC = cms.Sequence(photonsMCMatchForTable + photonMCTable)

