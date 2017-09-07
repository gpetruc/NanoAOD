import FWCore.ParameterSet.Config as cms
from  PhysicsTools.NanoAOD.common_cff import *



##################### User floats producers, selectors ##########################
from RecoJets.JetProducers.ak4PFJets_cfi import ak4PFJets

#chsForSATkJets = cms.EDFilter("CandPtrSelector", src = cms.InputTag("packedPFCandidates"), cut = cms.string('charge()!=0 && pvAssociationQuality()=="CompatibilityDz" && vertexRef().key()==0'))
chsForSATkJets = cms.EDFilter("CandPtrSelector", src = cms.InputTag("packedPFCandidates"), cut = cms.string('charge()!=0 && fromPV && vertexRef().key()==0'))
softActivityJets = ak4PFJets.clone(src = 'chsForSATkJets', doAreaFastjet = False, jetPtMin=1) 

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
        muon1 = Var("?overlaps('muons').size()>0?overlaps('muons')[0].key():-1", int, doc="index of first matching muon"),
        muon2 = Var("?overlaps('muons').size()>1?overlaps('muons')[1].key():-1", int, doc="index of second matching muon"),
        electron1 = Var("?overlaps('electrons').size()>0?overlaps('electrons')[0].key():-1", int, doc="index of first matching electron"),
        electron2 = Var("?overlaps('electrons').size()>1?overlaps('electrons')[1].key():-1", int, doc="index of second matching electron"),
        nElectrons = Var("?hasOverlaps('electrons')?overlaps('electrons').size():0", int, doc="number of electrons in the jet"),
	btagCMVA = Var("bDiscriminator('pfCombinedMVAV2BJetTags')",float,doc="CMVA V2 btag discriminator",precision=10),
	btagDeepB = Var("bDiscriminator('pfDeepCSVJetTags:probb')",float,doc="CMVA V2 btag discriminator",precision=10),
	btagDeepBB = Var("bDiscriminator('pfDeepCSVJetTags:probbb')",float,doc="CMVA V2 btag discriminator",precision=10),
	btagDeepC = Var("bDiscriminator('pfDeepCSVJetTags:probc')",float,doc="CMVA V2 btag discriminator",precision=10),
#puIdDisc = Var("userFloat('pileupJetId:fullDiscriminant')",float,doc="Pilup ID discriminant",precision=10),
	puId = Var("userInt('pileupJetId:fullId')",int,doc="Pilup ID flags"),
	qgl = Var("userFloat('QGTagger:qgLikelihood')",float,doc="Quark vs Gluon likelihood discriminator",precision=10),
	nConstituents = Var("numberOfDaughters()",int,doc="Number of particles in the jet"),
	rawFactor = Var("1.-jecFactor('Uncorrected')",float,doc="1 - Factor to get back to raw pT",precision=6),
        chHEF = Var("chargedHadronEnergy()/energy()", float, doc="charged Hadron Energy Fraction", precision= 6),
        neHEF = Var("neutralHadronEnergy()/energy()", float, doc="neutral Hadron Energy Fraction", precision= 6),
        chEmEF = Var("chargedEmEnergy()/energy()", float, doc="charged Electromagnetic Energy Fraction", precision= 6),
        neEmEF = Var("neutralEmEnergy()/energy()", float, doc="charged Electromagnetic EnergyFraction", precision= 6),

    )
)
#jets are not as precise as muons
jetTable.variables.pt.precision=10


bjetMVATable= cms.EDProducer("BJetEnergyRegressionMVATable",
    src = cms.InputTag("linkedObjects","jets"),
    pvsrc = cms.InputTag("offlineSlimmedPrimaryVertices"),
    weightFile =  cms.FileInPath("PhysicsTools/NanoAOD/data/bjet-regression.xml"),
    collName = cms.string("Jet"),
    attName = cms.string("bReg"),
    variablesOrder = cms.vstring(["Jet_pt","nPVs","Jet_eta","Jet_mt","Jet_leadTrackPt","Jet_leptonPtRel","Jet_leptonPt","Jet_leptonDeltaR","Jet_neHEF","Jet_neEmEF","Jet_vtxPt","Jet_vtxMass","Jet_vtx3dL","Jet_vtxNtrk","Jet_vtx3deL"]),
    variables = cms.PSet(
	Jet_pt = cms.string("pt"),
	Jet_eta = cms.string("eta"),
	Jet_mt = cms.string("mt"),
	Jet_leptonPt = cms.string("?overlaps('muons').size()>0?overlaps('muons')[0].pt():(?overlaps('electrons').size()>0?overlaps('electrons')[0].pt():0)"),
	Jet_neHEF = cms.string("neutralHadronEnergy()/energy()"),
	Jet_neEmEF = cms.string("neutralEmEnergy()/energy()"),
	Jet_leptonDeltaR = cms.string('''?overlaps('muons').size()>0?deltaR(eta,phi,overlaps('muons')[0].eta,overlaps('muons')[0].phi):
				(?overlaps('electrons').size()>0?deltaR(eta,phi,overlaps('electrons')[0].eta,overlaps('electrons')[0].phi):
				0)'''),
	#Jet_leptonPtRel = cms.string("?overlaps('muons').size()>0?overlaps('muons')[0].userFloat('ptRel'):(?overlaps('electrons').size()>0?overlaps('electrons')[0].userFloat('ptRel'):-1)"),
	Jet_vtxPt = cms.string("pt"),
	Jet_vtxMass = cms.string("pt"),
	Jet_vtx3dL = cms.string("pt"),
	Jet_vtxNtrk = cms.string("pt"),
	Jet_vtx3deL = cms.string("pt"),
    )

)

##### Soft Activity tables
saJetTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = cms.InputTag("softActivityJets"),
    cut = cms.string(""),
    maxLen = cms.uint32(6),
    name = cms.string("SoftActivityJet"),
    doc  = cms.string("jets clustered from charged candidates compatible with primary vertex (" + chsForSATkJets.cut.value()+")"),
    singleton = cms.bool(False), # the number of entries is variable
    extension = cms.bool(False), # this is the main table for the jets
    variables = cms.PSet(P3Vars,
  )
)

saJet2Table = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = cms.InputTag("softActivityJets"),
    cut = cms.string("pt>2"),
    name = cms.string("SoftActivityJetPt2"),
    doc  = cms.string("number of sa jets with pt >2, clustered from charged candidates compatible with primary vertex (" + chsForSATkJets.cut.value()+")"),
    singleton = cms.bool(False), # the number of entries is variable
    extension = cms.bool(False), # this is the main table for the jets
    variables = cms.PSet()
)
saJet5Table = saJet2Table.clone(
	cut=cms.string("pt>5"),
	doc  = cms.string("number of sa jets with pt >5,  clustered from charged candidates compatible with primary vertex (" + chsForSATkJets.cut.value()+")"),
	name = cms.string("SoftActivityJetPt5"),
)
saJet10Table = saJet2Table.clone(
	cut=cms.string("pt>10"),
	doc  = cms.string("number of sa jets with pt >10,  clustered from charged candidates compatible with primary vertex (" + chsForSATkJets.cut.value()+")"),
	name = cms.string("SoftActivityJetPt10"),
)



saJetTable.variables.pt.precision=10
saJetTable.variables.eta.precision=8
saJetTable.variables.phi.precision=8

saTable = cms.EDProducer("GlobalVariablesTableProducer",
    variables = cms.PSet(
        SoftActivityJetHT = ExtVar( cms.InputTag("softActivityJets"), "candidatescalarsum", doc = "scalar sum of soft activity jet pt" ),
    )
)



## BOOSTED STUFF #################
fatJetTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = cms.InputTag("slimmedJetsAK8"),
    cut = cms.string(" pt > 170"), #probably already applied in miniaod
    name = cms.string("FatJet"),
    doc  = cms.string("slimmedJetsAK8, i.e. ak8 fat jets for boosted analysis"),
    singleton = cms.bool(False), # the number of entries is variable
    extension = cms.bool(False), # this is the main table for the jets
    variables = cms.PSet(P4Vars,
        area = Var("jetArea()", float, doc="jet catchment area, for JECs",precision=10),
        tau1 = Var("userFloat('ak8PFJetsCHSValueMap:NjettinessAK8CHSTau1')",float, doc="Nsubjettiness (1 axis)",precision=10),
        tau2 = Var("userFloat('ak8PFJetsCHSValueMap:NjettinessAK8CHSTau2')",float, doc="Nsubjettiness (2 axis)",precision=10),
        tau3 = Var("userFloat('ak8PFJetsCHSValueMap:NjettinessAK8CHSTau3')",float, doc="Nsubjettiness (3 axis)",precision=10),
        msoftdrop = Var("userFloat('ak8PFJetsCHSValueMap:ak8PFJetsCHSSoftDropMass')",float, doc="Soft drop mass",precision=10),
        mpruned = Var("userFloat('ak8PFJetsCHSValueMap:ak8PFJetsCHSPrunedMass')", float, doc="Pruned mass",precision=10),


        btagCMVA = Var("bDiscriminator('pfCombinedMVAV2BJetTags')",float,doc="CMVA V2 btag discriminator",precision=10),
        btagDeepB = Var("bDiscriminator('pfDeepCSVJetTags:probb')",float,doc="CMVA V2 btag discriminator",precision=10),
        btagDeepBB = Var("bDiscriminator('pfDeepCSVJetTags:probbb')",float,doc="CMVA V2 btag discriminator",precision=10),

        subJet1 = Var("?numberOfSourceCandidatePtrs()>0 && sourceCandidatePtr(0).numberOfSourceCandidatePtrs()>0?sourceCandidatePtr(0).key():-1", int,
		     doc="index of first subjet"),
        subJet2 = Var("?numberOfSourceCandidatePtrs()>1 && sourceCandidatePtr(1).numberOfSourceCandidatePtrs()>0?sourceCandidatePtr(1).key():-1", int,
		     doc="index of second subjet"),
        subJet3 = Var("?numberOfSourceCandidatePtrs()>2 && sourceCandidatePtr(2).numberOfSourceCandidatePtrs()>0?sourceCandidatePtr(2).key():-1", int,
		     doc="index of third subjet"),
	
#        btagDeepC = Var("bDiscriminator('pfDeepCSVJetTags:probc')",float,doc="CMVA V2 btag discriminator",precision=10),
#puIdDisc = Var("userFloat('pileupJetId:fullDiscriminant')",float,doc="Pilup ID discriminant",precision=10),
#        nConstituents = Var("numberOfDaughters()",int,doc="Number of particles in the jet"),
#        rawFactor = Var("1.-jecFactor('Uncorrected')",float,doc="1 - Factor to get back to raw pT",precision=6),
    )
)

subJetTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = cms.InputTag("slimmedJetsAK8PFPuppiSoftDropPacked","SubJets"),
    cut = cms.string(""), #probably already applied in miniaod
    name = cms.string("SubJet"),
    doc  = cms.string("slimmedJetsAK8, i.e. ak8 fat jets for boosted analysis"),
    singleton = cms.bool(False), # the number of entries is variable
    extension = cms.bool(False), # this is the main table for the jets
    variables = cms.PSet(P4Vars,
        btagCMVA = Var("bDiscriminator('pfCombinedMVAV2BJetTags')",float,doc="CMVA V2 btag discriminator",precision=10),
        btagDeepB = Var("bDiscriminator('pfDeepCSVJetTags:probb')",float,doc="CMVA V2 btag discriminator",precision=10),
        btagDeepBB = Var("bDiscriminator('pfDeepCSVJetTags:probbb')",float,doc="CMVA V2 btag discriminator",precision=10),
    )
)

#jets are not as precise as muons
fatJetTable.variables.pt.precision=10







## MC STUFF ######################
jetMCTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = cms.InputTag("linkedObjects","jets"),
    cut = cms.string(""), #we should not filter on cross linked collections
    name = cms.string("Jet"),
    singleton = cms.bool(False), # the number of entries is variable
    extension = cms.bool(True), # this is an extension  table for the jets
    variables = cms.PSet(
        partonFlavour = Var("partonFlavour()", int, doc="flavour from parton matching"),
        hadroFlavour = Var("hadronFlavour()", int, doc="flavour from hadron ghost clustering"),
        genJetIdx = Var("?genJetFwdRef().backRef().isNonnull()?genJetFwdRef().backRef().key():-1", int, doc="index of matched gen jet"),
    )
)
genJetTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = cms.InputTag("slimmedGenJets"),
    cut = cms.string("pt > 10"),
    name = cms.string("GenJet"),
    doc  = cms.string("slimmedGenJets, i.e. ak4 Jets made with visible genparticles"),
    singleton = cms.bool(False), # the number of entries is variable
    extension = cms.bool(False), # this is the main table for the genjets
    variables = cms.PSet(P4Vars,
	#anything else?
    )
)



#before cross linking
jetSequence = cms.Sequence(chsForSATkJets+softActivityJets+finalJets)
#after cross linkining
jetTables = cms.Sequence( jetTable+fatJetTable+subJetTable+saJetTable+saJet2Table+saJet5Table+saJet10Table+bjetMVATable+saTable)

#MC only producers and tables
jetMC = cms.Sequence(jetMCTable+genJetTable)

