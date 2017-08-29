import FWCore.ParameterSet.Config as cms
from PhysicsTools.NanoAOD.common_cff import *

finalElectrons = cms.EDFilter("PATElectronRefSelector",
    src = cms.InputTag("slimmedElectrons"),
    cut = cms.string("pt > 5 ")
)

electronTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = cms.InputTag("linkedObjects","electrons"),
    cut = cms.string(""), #we should not filter on cross linked collections
    name= cms.string("Electron"),
    singleton = cms.bool(False),
    variables = cms.PSet(CandVars,
       jet = Var("?hasUserCand('jet')?userCand('jet').key():-1", int, doc="index of the associated jet (-1 if none)"),
    )
)

electronSequence = cms.Sequence(finalElectrons)
electronTables = cms.Sequence ( electronTable)

