import FWCore.ParameterSet.Config as cms
from  PhysicsTools.NanoAOD.common_cff import *



##################### User floats producers, selectors ##########################


finalJets = cms.EDFilter("PATJetRefSelector",
    src = cms.InputTag("slimmedJets"),
    cut = cms.string("pt > 15")
)

##################### Tables for final output and docs ##########################
jetTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = cms.InputTag("linkedObjects","jets"),
    cut = cms.string(""), #we should not filter on cross linked collections
    name = cms.string("Jet"),
    singleton = cms.bool(False),
    variables = cms.PSet(P4Vars,
        area = Var("jetArea()", float, doc="jet catchment area, for JECs"),
        nMuonsInJet = Var("?hasOverlaps('muons')?overlaps('muons').size():0", int, doc="number of muons in the jet"),
    )
)

jetSequence = cms.Sequence(finalJets)
jetTables = cms.Sequence( jetTable )

