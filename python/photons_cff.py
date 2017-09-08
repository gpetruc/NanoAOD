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
       cutBased = Var("userInt('cutbasedID_loose')+userInt('cutbasedID_medium')+userInt('cutbasedID_tight')",int,doc="cut-based ID (0:fail, 1::loose, 2:medium, 3:tight)"),
       VIDNestedWPBitmap = Var("userInt('VIDNestedWPBitmap')",int),
       electronVeto = Var("passElectronVeto()",bool,doc="pass electron veto"),
       pixelSeed = Var("hasPixelSeed()",bool,doc="has pixel seed"),
       mvaID = Var("userFloat('mvaID')",float,doc="MVA ID score",precision=10),
       mvaID_WP90 = Var("userInt('mvaID_WP90')",bool,doc="MVA ID WP90"),
       mvaID_WP80 = Var("userInt('mvaID_WP90')",bool,doc="MVA ID WP80"),
       PFIso03_chg = Var("userFloat('PFIsoChg')",float,doc="PF isolation dR=0.3, charged component (with rho*EA PU corrections)"),
       PFIso03_all = Var("userFloat('PFIsoAll')",float,doc="PF isolation dR=0.3, total (with rho*EA PU corrections)"),
       hoe = Var("hadronicOverEm()",float,doc="H over E",precision=8),
    )
)

from PhysicsTools.SelectorUtils.tools.vid_id_tools import setupVIDSelection
from RecoEgamma.PhotonIdentification.egmPhotonIDs_cfi import *
from RecoEgamma.PhotonIdentification.PhotonIDValueMapProducer_cfi import *
from RecoEgamma.PhotonIdentification.PhotonMVAValueMapProducer_cfi import *
from RecoEgamma.PhotonIdentification.PhotonRegressionValueMapProducer_cfi import *
from RecoEgamma.EgammaIsolationAlgos.egmPhotonIsolationMiniAOD_cff import *
egmPhotonIDSequence = cms.Sequence(cms.Task(egmPhotonIsolationMiniAODTask,photonIDValueMapProducer,photonMVAValueMapProducer,egmPhotonIDs,photonRegressionValueMapProducer))
egmPhotonIDs.physicsObjectIDs = cms.VPSet()
egmPhotonIDs.physicsObjectSrc = cms.InputTag('slimmedPhotons')
photon_id_vid_modules=[
'RecoEgamma.PhotonIdentification.Identification.cutBasedPhotonID_Spring16_V2p2_cff',
'RecoEgamma.PhotonIdentification.Identification.mvaPhotonID_Spring16_nonTrig_V1_cff',
]
for modname in photon_id_vid_modules: 
    ids= __import__(modname, globals(), locals(), ['idName','cutFlow'])
    for name in dir(ids):
        _id = getattr(ids,name)
        if hasattr(_id,'idName') and hasattr(_id,'cutFlow'):
            setupVIDSelection(egmPhotonIDs,_id)
            if (_id.idName==cms.string('cutBasedPhotonID-Spring16-V2p2-loose')):
                photonTable.variables.VIDNestedWPBitmap.doc = cms.string('VID compressed bitmap (%s)'%(' '.join([cut.cutName.value() for cut in _id.cutFlow]),))

slimmedPhotonsWithUserData = cms.EDProducer("PATPhotonUserDataEmbedder",
                                            src = cms.InputTag("slimmedPhotons"),
                                            userFloats = cms.PSet(
            mvaID = cms.InputTag("photonMVAValueMapProducer:PhotonMVAEstimatorRun2Spring16NonTrigV1Values"),
            PFIsoChg = cms.InputTag("isoForPho:PFIsoChg"),
            PFIsoAll = cms.InputTag("isoForPho:PFIsoAll"),
            ),
                                            userIntFromBools = cms.PSet(
            cutbasedID_loose = cms.InputTag("egmPhotonIDs:cutBasedPhotonID-Spring16-V2p2-loose"),
            cutbasedID_medium = cms.InputTag("egmPhotonIDs:cutBasedPhotonID-Spring16-V2p2-medium"),
            cutbasedID_tight = cms.InputTag("egmPhotonIDs:cutBasedPhotonID-Spring16-V2p2-tight"),
            mvaID_WP90 = cms.InputTag("egmPhotonIDs:mvaPhoID-Spring16-nonTrig-V1-wp90"),
            mvaID_WP80 = cms.InputTag("egmPhotonIDs:mvaPhoID-Spring16-nonTrig-V1-wp80"),
            ),
                                            userInts = cms.PSet(
            VIDNestedWPBitmap = cms.InputTag("bitmapVIDForPho:VIDNestedWPBitmap"),
            ),
                                            )

bitmapVIDForPho = cms.EDProducer("PhoVIDNestedWPBitmapProducer",
                                 src = cms.InputTag("slimmedPhotons"),
                                 WorkingPoints = cms.vstring(
        "egmPhotonIDs:cutBasedPhotonID-Spring16-V2p2-loose",
        "egmPhotonIDs:cutBasedPhotonID-Spring16-V2p2-medium",
        "egmPhotonIDs:cutBasedPhotonID-Spring16-V2p2-tight",
        )
                                 )

isoForPho = cms.EDProducer("PhoIsoValueMapProducer",
                           src = cms.InputTag("slimmedPhotons"),
                           rho = cms.InputTag("fixedGridRhoFastjetAll"),
                           mapIsoChg = cms.InputTag("photonIDValueMapProducer:phoChargedIsolation"),
                           mapIsoNeu = cms.InputTag("photonIDValueMapProducer:phoNeutralHadronIsolation"),
                           mapIsoPho = cms.InputTag("photonIDValueMapProducer:phoPhotonIsolation"),
                           EAFile_PFIso_Chg = cms.FileInPath("RecoEgamma/PhotonIdentification/data/Spring16/effAreaPhotons_cone03_pfChargedHadrons_90percentBased.txt"),
                           EAFile_PFIso_Neu = cms.FileInPath("RecoEgamma/PhotonIdentification/data/Spring16/effAreaPhotons_cone03_pfNeutralHadrons_90percentBased.txt"),
                           EAFile_PFIso_Pho = cms.FileInPath("RecoEgamma/PhotonIdentification/data/Spring16/effAreaPhotons_cone03_pfPhotons_90percentBased.txt"),
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

photonSequence = cms.Sequence(egmPhotonIDSequence + bitmapVIDForPho + isoForPho + slimmedPhotonsWithUserData + finalPhotons)
photonTables = cms.Sequence ( photonTable)
photonMC = cms.Sequence(photonsMCMatchForTable + photonMCTable)

