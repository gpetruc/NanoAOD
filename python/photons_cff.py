import FWCore.ParameterSet.Config as cms
from PhysicsTools.NanoAOD.common_cff import *

finalPhotons = cms.EDFilter("PATPhotonRefSelector",
    src = cms.InputTag("slimmedPhotonsWithUserData"),
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
       electron = Var("?hasUserCand('electron')?userCand('electron').key():-1", int, doc="index of the associated electron (-1 if none)"),
       r9 = Var("full5x5_r9()",float,doc="R9 of the supercluster, calculated with full 5x5 region",precision=10),
       sieie = Var("full5x5_sigmaIetaIeta()",float,doc="sigma_IetaIeta of the supercluster, calculated with full 5x5 region",precision=10),
#       cutBased = Var("userInt('cutbasedID_loose')+userInt('cutbasedID_medium')+userInt('cutbasedID_tight')",int,doc="cut-based ID (0:fail, 1::loose, 2:medium, 3:tight)"),
       electronVeto = Var("passElectronVeto()",bool,doc="pass electron veto"),
       pixelSeed = Var("hasPixelSeed()",bool,doc="has pixel seed"),
    )
)


# ADD egmPhotonIDSequence when uncomment
#
#from PhysicsTools.SelectorUtils.tools.vid_id_tools import setupVIDSelection
#from RecoEgamma.PhotonIdentification.egmPhotonIDs_cfi import *
#from RecoEgamma.PhotonIdentification.PhotonIDValueMapProducer_cfi import *
#from RecoEgamma.PhotonIdentification.PhotonMVAValueMapProducer_cfi import *
#from RecoEgamma.PhotonIdentification.PhotonRegressionValueMapProducer_cfi import *
#from RecoEgamma.EgammaIsolationAlgos.egmPhotonIsolationMiniAOD_cff import *
#egmPhotonIDSequence = cms.Sequence(cms.Task(egmPhotonIsolationMiniAODTask,photonIDValueMapProducer,photonMVAValueMapProducer,egmPhotonIDs,photonRegressionValueMapProducer))
#egmPhotonIDs.physicsObjectIDs = cms.VPSet()
#egmPhotonIDs.physicsObjectSrc = cms.InputTag('slimmedPhotons')
#photon_id_vid_modules=[
#'RecoEgamma.PhotonIdentification.Identification.cutBasedPhotonID_Spring16_V2p2_cff',
#]
#for modname in photon_id_vid_modules: 
#    ids= __import__(modname, globals(), locals(), ['idName','cutFlow'])
#    for name in dir(ids):
#        _id = getattr(ids,name)
#        if hasattr(_id,'idName') and hasattr(_id,'cutFlow'):
#            setupVIDSelection(egmPhotonIDs,_id)

slimmedPhotonsWithUserData = cms.EDProducer("PATPhotonUserDataEmbedder",
                                            src = cms.InputTag("slimmedPhotons"),
                                            userIntFromBools = cms.PSet(
#            cutbasedID_loose = cms.InputTag("egmPhotonIDs:cutBasedPhotonID-Spring16-V2p2-loose"),
#            cutbasedID_medium = cms.InputTag("egmPhotonIDs:cutBasedPhotonID-Spring16-V2p2-medium"),
#            cutbasedID_tight = cms.InputTag("egmPhotonIDs:cutBasedPhotonID-Spring16-V2p2-tight"),
            ),
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

photonSequence = cms.Sequence(slimmedPhotonsWithUserData + finalPhotons)
photonTables = cms.Sequence ( photonTable)
photonMC = cms.Sequence(photonsMCMatchForTable + photonMCTable)

