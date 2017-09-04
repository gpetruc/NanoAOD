import FWCore.ParameterSet.Config as cms
from PhysicsTools.NanoAOD.common_cff import *

finalElectrons = cms.EDFilter("PATElectronRefSelector",
    src = cms.InputTag("slimmedElectronsWithUserData"),
    cut = cms.string("pt > 5 ")
)

#electron_emuIdHLT_cut_E2 = "(hadronicOverEm()<(0.10-0.03*(?(abs(superCluster().eta())>1.479)?1:0))) && (abs(deltaEtaSuperClusterTrackAtVtx())<(0.01-0.002*(?(abs(superCluster().eta())>1.479)?1:0))) && (abs(deltaPhiSuperClusterTrackAtVtx())<(0.04+0.03*(?(abs(superCluster().eta())>1.479)?1:0))) && ((?(ecalEnergy()>0.)?(1.0/ecalEnergy() - eSuperClusterOverP()/ecalEnergy()):9e9)>-0.05) && ((?(ecalEnergy()>0.)?(1.0/ecalEnergy() - eSuperClusterOverP()/ecalEnergy()):9e9)<(0.01-0.005*(?(abs(superCluster().eta())>1.479)?1:0))) && (full5x5_sigmaIetaIeta()<(0.011+0.019*(?(abs(superCluster().eta())>1.479)?1:0)))" # to be debugged

electronTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = cms.InputTag("linkedObjects","electrons"),
    cut = cms.string(""), #we should not filter on cross linked collections
    name= cms.string("Electron"),
    doc = cms.string("slimmedElectrons after basic selection (" + finalElectrons.cut.value()+")"),
    singleton = cms.bool(False), # the number of entries is variable
    extension = cms.bool(False), # this is the main table for the electrons
    variables = cms.PSet(CandVars,
       jet = Var("?hasUserCand('jet')?userCand('jet').key():-1", int, doc="index of the associated jet (-1 if none)"),
#       emuIdHLT = Var(electron_emuIdHLT_cut_E2,bool,doc="passes emulated HLT electron ID (E2 version)"),
       pterr = Var("gsfTrack().ptError()",float,doc="pt error of the GSF track"),
       dxy = Var("dB('PV2D')",float,doc="dxy (with sign)"),
       sip3d = Var("abs(dB('PV3D')/edB('PV3D'))",float,doc="SIP_3D"),
       deltaEtaSC = Var("superCluster().eta()-eta()",float,doc="delta eta (SC,ele) with sign"),
       r9 = Var("full5x5_r9()",float,doc="R9 of the supercluster, calculated with full 5x5 region"),
       sieie = Var("full5x5_sigmaIetaIeta()",float,doc="sigma_IetaIeta of the supercluster, calculated with full 5x5 region"),
       mvaSpring16GP = Var("userFloat('mvaSpring16GP')",float,doc="MVA general-purpose ID score"),
       mvaSpring16GP_WP80 = Var("userInt('mvaSpring16GP_WP80')",int,doc="MVA general-purpose ID WP80"),
       mvaSpring16GP_WP90 = Var("userInt('mvaSpring16GP_WP90')",int,doc="MVA general-purpose ID WP90"),
       mvaSpring16HZZ = Var("userFloat('mvaSpring16HZZ')",float,doc="MVA HZZ ID score"),
       mvaSpring16HZZ_WPL = Var("userInt('mvaSpring16HZZ_WPL')",int,doc="MVA HZZ ID loose WP"),
       cutBased = Var("userInt('cutbasedID_veto')+userInt('cutbasedID_loose')+userInt('cutbasedID_medium')+userInt('cutbasedID_tight')",int,doc="cut-based ID (0:fail, 1:veto, 2:loose, 3:medium, 4:tight)"),
       cutBased_HLTPreSel = Var("userInt('cutbasedID_HLT')",int,doc="cut-based HLT pre-selection ID"),
    )
)


from PhysicsTools.SelectorUtils.tools.vid_id_tools import setupVIDSelection
from RecoEgamma.ElectronIdentification.egmGsfElectronIDs_cff import *
egmGsfElectronIDs.physicsObjectIDs = cms.VPSet()
egmGsfElectronIDs.physicsObjectSrc = cms.InputTag('slimmedElectrons')
electron_id_vid_modules=[
'RecoEgamma.ElectronIdentification.Identification.cutBasedElectronID_Summer16_80X_V1_cff',
'RecoEgamma.ElectronIdentification.Identification.cutBasedElectronHLTPreselecition_Summer16_V1_cff',
#'RecoEgamma.ElectronIdentification.Identification.heepElectronID_HEEPV70_cff', # requires further adaptation in calling code
'RecoEgamma.ElectronIdentification.Identification.mvaElectronID_Spring16_GeneralPurpose_V1_cff',
'RecoEgamma.ElectronIdentification.Identification.mvaElectronID_Spring16_HZZ_V1_cff',
]
for modname in electron_id_vid_modules: 
    ids= __import__(modname, globals(), locals(), ['idName','cutFlow'])
    for name in dir(ids):
        _id = getattr(ids,name)
        if hasattr(_id,'idName') and hasattr(_id,'cutFlow'):
            setupVIDSelection(egmGsfElectronIDs,_id)

slimmedElectronsWithUserData = cms.EDProducer("PATElectronUserDataEmbedder",
                                        src = cms.InputTag("slimmedElectrons"),
                                        userFloats = cms.PSet(
        mvaSpring16GP = cms.InputTag("electronMVAValueMapProducer:ElectronMVAEstimatorRun2Spring16GeneralPurposeV1Values"),
        mvaSpring16HZZ = cms.InputTag("electronMVAValueMapProducer:ElectronMVAEstimatorRun2Spring16HZZV1Values"),
        ),
                                              userIntFromBools = cms.PSet(
        mvaSpring16GP_WP90 = cms.InputTag("egmGsfElectronIDs:mvaEleID-Spring16-GeneralPurpose-V1-wp90"),
        mvaSpring16GP_WP80 = cms.InputTag("egmGsfElectronIDs:mvaEleID-Spring16-GeneralPurpose-V1-wp80"),
        mvaSpring16HZZ_WPL = cms.InputTag("egmGsfElectronIDs:mvaEleID-Spring16-HZZ-V1-wpLoose"),
        cutbasedID_veto = cms.InputTag("egmGsfElectronIDs:cutBasedElectronID-Summer16-80X-V1-veto"),
        cutbasedID_loose = cms.InputTag("egmGsfElectronIDs:cutBasedElectronID-Summer16-80X-V1-loose"),
        cutbasedID_medium = cms.InputTag("egmGsfElectronIDs:cutBasedElectronID-Summer16-80X-V1-medium"),
        cutbasedID_tight = cms.InputTag("egmGsfElectronIDs:cutBasedElectronID-Summer16-80X-V1-tight"),
        cutbasedID_HLT = cms.InputTag("egmGsfElectronIDs:cutBasedElectronHLTPreselection-Summer16-V1"),
        ),
                                              )

electronSequence = cms.Sequence(egmGsfElectronIDSequence + slimmedElectronsWithUserData + finalElectrons)
electronTables = cms.Sequence ( electronTable)
