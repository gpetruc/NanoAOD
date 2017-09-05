import FWCore.ParameterSet.Config as cms
from  PhysicsTools.NanoAOD.common_cff import *



##################### User floats producers, selectors ##########################


finalTaus = cms.EDFilter("PATTauRefSelector",
    src = cms.InputTag("slimmedTaus"),
    cut = cms.string("pt > 18 && tauID('decayModeFindingNewDMs')")
)

##################### Tables for final output and docs ##########################
tauTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = cms.InputTag("linkedObjects","taus"),
    cut = cms.string(""), #we should not filter on cross linked collections
    name= cms.string("Tau"),
    doc = cms.string("slimmedTaus after basic selection (" + finalTaus.cut.value()+")"),
    singleton = cms.bool(False), # the number of entries is variable
    extension = cms.bool(False), # this is the main table for the taus
    variables = cms.PSet(P4Vars,
       jet = Var("?hasUserCand('jet')?userCand('jet').key():-1", int, doc="index of the associated jet (-1 if none)"),
       decayMode = Var("decayMode()",int),
       idDecayMode = Var("tauID('decayModeFinding')", int),
       idDecayModeNewDMs = Var("tauID('decayModeFindingNewDMs')", int),
       dxy = Var("dxy()",float, doc="d_{xy} of lead track with respect to PV, in cm (with sign)",precision=10),
       dz = Var("leadChargedHadrCand().dz() ",float, doc="d_{z} of lead track with respect to PV, in cm (with sign)",precision=14),
       rawMVArun2 = Var( "tauID('byIsolationMVArun2v1DBoldDMwLTraw')",float, doc="byIsolationMVArun2v1DBoldDMwLT raw output discriminator",precision=10),
#       rawMVArun2dR03 = Var( 'tauID("byIsolationMVArun2v1DBdR03oldDMwLTraw")',float,doc="byIsolationMVArun2v1DBdR03oldDMwLT raw output discriminator"),
#       rawMVArun2NewDM = Var( "tauID('byIsolationMVArun2v1DBnewDMwLTraw')",float,doc="byIsolationMVArun2v1DBnewDMwLT raw output discriminator"),

 #      idMVArun2NewDM = Var( "idMVArun2NewDM", int, doc="1,2,3,4,5,6 if the tau passes the very loose to very very tight WP of the MVArun2v1DBnewDMwLT discriminator"),
#       idMVArun2 = Var( "idMVArun2" ,int, doc="1,2,3,4,5,6 if the tau passes the very loose to very very tight WP of the MVArun2v1DBoldDMwLT discriminator"),
#       idMVArun2dR03 = Var( 'idMVArun2dR03', int, doc="1,2,3,4,5,6 if the tau passes the very loose to very very tight WP of the MVArun2v1DBdR03oldDMwLT discriminator"),
#   idCI3hit = Var( "idCI3hit" int, doc="1,2,3 if the tau passes the loose, medium, tight WP of the By<X>CombinedIsolationDBSumPtCorr3Hits discriminator"),
#    idCI3hitdR03 = Var( "idCI3hitdR03" int, doc="1,2,3 if the tau passes the loose, medium, tight WP of the By<X>CombinedIsolationDeltaBetaCorr3HitsdR03 discriminator"),
#   idAntiMu = Var( "idAntiMu" int, doc="1,2 if the tau passes the loose/tight WP of the againstMuon<X>3 discriminator"),
#    idAntiE = Var( "idAntiE" int, doc="1,2,3,4,5 if the tau passes the v loose, loose, medium, tight, v tight WP of the againstElectron<X>MVA5 discriminator"),
#   idAntiErun2 = Var( "idAntiErun2" int, doc="1,2,3,4,5 if the tau passes the v loose, loose, medium, tight, v tight WP of the againstElectron<X>MVA6 discriminator"),
#   isoCI3hit = Var(  "tauID("byCombinedIsolationDeltaBetaCorrRaw3Hits")" doc="byCombinedIsolationDeltaBetaCorrRaw3Hits raw output discriminator"),
#   photonOutsideSigCone = Var( "tauID("photonPtSumOutsideSignalCone")" doc="photonPtSumOutsideSignalCone raw output discriminator"),


    )
)

tausMCMatchForTable = cms.EDProducer("MCMatcher",  # cut on deltaR, deltaPt/Pt; pick best by deltaR
    src         = tauTable.src,                 # final reco collection
    matched     = cms.InputTag("finalGenParticles"), # final mc-truth particle collection
    mcPdgId     = cms.vint32(15),                    # one or more PDG ID (15 = tau); absolute values (see below)
    checkCharge = cms.bool(False),              # True = require RECO and MC objects to have the same charge
    mcStatus    = cms.vint32(2),                # PYTHIA status code (1 = stable, 2 = shower, 3 = hard scattering)
    maxDeltaR   = cms.double(0.4),              # Minimum deltaR for the match
    maxDPtRel   = cms.double(0.3),              # Minimum deltaPt/Pt for the match
    resolveAmbiguities    = cms.bool(True),     # Forbid two RECO objects to match to the same GEN object
    resolveByMatchQuality = cms.bool(True),    # False = just match input in order; True = pick lowest deltaR pair first
)

tauMCTable = cms.EDProducer("CandMCMatchTableProducer",
    src     = tauTable.src,
    mcMap   = cms.InputTag("tausMCMatchForTable"),
    objName = tauTable.name,
    objType = tauTable.name, #cms.string("Tau"),
    branchName = cms.string("mcMatch"),
    docString = cms.string("MC matching to status==2 taus"),
)


tauSequence = cms.Sequence(finalTaus)
tauTables = cms.Sequence( tauTable )
tauMC = cms.Sequence(tausMCMatchForTable + tauMCTable)

