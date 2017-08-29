import FWCore.ParameterSet.Config as cms
from  PhysicsTools.NanoAOD.common_cff import *



##################### User floats producers, selectors ##########################


finalTaus = cms.EDFilter("PATTauRefSelector",
    src = cms.InputTag("slimmedTaus"),
    cut = cms.string("pt > 15")
)

##################### Tables for final output and docs ##########################
tauTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = cms.InputTag("linkedObjects","taus"),
    cut = cms.string(""), #we should not filter on cross linked collections
    name= cms.string("Tau"),
    singleton = cms.bool(False),
    variables = cms.PSet(CandVars,
       jet = Var("?hasUserCand('jet')?userCand('jet').key():-1", int, doc="index of the associated jet (-1 if none)"),
    )
)

tauSequence = cms.Sequence(finalTaus)
tauTables = cms.Sequence( tauTable )

