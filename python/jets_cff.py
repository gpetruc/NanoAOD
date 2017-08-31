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
    doc  = cms.string("slimmedJets, i.e. ak4 PFJets CHS with JECs applied, after basic selection (" + finalJets.cut.value()+")"),
    singleton = cms.bool(False), # the number of entries is variable
    extension = cms.bool(False), # this is the main table for the jets
    variables = cms.PSet(P4Vars,
        area = Var("jetArea()", float, doc="jet catchment area, for JECs",precision=10),
        nMuons = Var("?hasOverlaps('muons')?overlaps('muons').size():0", int, doc="number of muons in the jet"),
        nElectrons = Var("?hasOverlaps('electrons')?overlaps('electrons').size():0", int, doc="number of electrons in the jet"),
	btagCMVA = Var("bDiscriminator('pfCombinedMVAV2BJetTags')",float,doc="CMVA V2 btag discriminator",precision=10),
	puIdDisc = Var("userFloat('pileupJetId:fullDiscriminant')",float,doc="Pilup ID discriminant"),
	qgl = Var("userFloat('QGTagger:qgLikelihood')",float,doc="Quark vs Gluon likelihood discriminator",precision=10),
	nConstituents = Var("numberOfDaughters()",float,doc="Number of particles in the jet"),
	rawFactor = Var("1.-jecFactor('Uncorrected')",float,doc="1 - Factor to get back to raw pT",precision=8),
    )
)

jetMCTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = cms.InputTag("linkedObjects","jets"),
    cut = cms.string(""), #we should not filter on cross linked collections
    name = cms.string("Jet"),
    singleton = cms.bool(False), # the number of entries is variable
    extension = cms.bool(True), # this is an extension  table for the jets
    variables = cms.PSet(
        partonFlavour = Var("partonFlavour()", float, doc="flavour from parton matching"),
        hadroFlavour = Var("hadronFlavour()", float, doc="flavour from hadron ghost clustering"),
    )
)

#before cross linking
jetSequence = cms.Sequence(finalJets)
jetSequenceMC = cms.Sequence(jetSequence)
#after cross linkining
jetTables = cms.Sequence( jetTable)
jetTablesMC = cms.Sequence( jetTables+jetMCTable)

