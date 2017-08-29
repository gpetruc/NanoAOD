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
    singleton = cms.bool(False),
    variables = cms.PSet(CandVars,
       jet = Var("?hasUserCand('jet')?userCand('jet').key():-1", int, doc="index of the associated jet (-1 if none)"),
    )
)

photonSequence = cms.Sequence(finalPhotons)
photonTables = cms.Sequence ( photonTable)

