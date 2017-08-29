import FWCore.ParameterSet.Config as cms

def Var(expr, valtype, compression=None, doc=None, mcOnly=False):
    """ Create a PSet for a new variable

           expr is the expression to use
           valtype is the type of the value (float, int, bool, or a string that the table producer understands),
           compression is not currently used,
           doc is a docstring, that will be passed to the producer,
           mcOnly can be set to True for variables that exist only in MC samples and not in data ones. 
    """
    if   valtype == float: valtype = "float"
    elif valtype == int:   valtype = "int"
    elif valtype == bool:  valtype = "bool"
    return cms.PSet( 
                expr = cms.string(expr),
                type = cms.string(valtype),
                compression = cms.string(compression if compression else "none"),
                doc = cms.string(doc if doc else expr),
                mcOnly = cms.bool(mcOnly),
           )

PTVars = cms.PSet(
    pt  = Var("pt",  float), 
    phi = Var("phi", float), 
)
P4Vars = cms.PSet(PTVars,
    eta  = Var("eta",  float), 
    mass = Var("mass", float), 
)
CandVars = cms.PSet(P4Vars,
    pdgId  = Var("pdgId", int, doc="PDG code assigned by the event reconstruction (not by MC truth)"),
    charge = Var("charge", int, doc="electric charge"),
)

finalMuons = cms.EDFilter("PATMuonRefSelector",
    src = cms.InputTag("slimmedMuons"),
    cut = cms.string("pt > 5 && track.isNonnull && (isGlobalMuon || isTrackerMuon) && isPFMuon")
)

finalJets = cms.EDFilter("PATJetRefSelector",
    src = cms.InputTag("slimmedJets"),
    cut = cms.string("pt > 15")
)

linkedObjects = cms.EDProducer("PATObjectCrossLinker",
   jets=cms.InputTag("finalJets"),
   muons=cms.InputTag("finalMuons"),
   electrons=cms.InputTag("slimmedElectrons"),
   taus=cms.InputTag("slimmedTaus"),
   photons=cms.InputTag("slimmedPhotons"),
)

jetTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = cms.InputTag("linkedObjects","jets"),
    cut = cms.string(""), #we should not filter on cross linked collections
    name = cms.string("Jet"),
    singleton = cms.bool(False),
    vars = cms.PSet(P4Vars,
        area = Var("jetArea()", float, doc="jet catchment area, for JECs"),
        nMuonsInJet = Var("?hasOverlaps('muons')?overlaps('muons').size():0", int, doc="number of muons in the jet"),
    )
)

muonTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = cms.InputTag("linkedObjects","muons"),
    cut = cms.string(""), #we should not filter on cross linked collections
    name = cms.string("Muon"),
    singleton = cms.bool(False),
    vars = cms.PSet(CandVars,
        dxy   = Var("dB", float, doc = "dxy wrt first PV, in cm"), 
        jetPt = Var("?hasUserCand('jet')?userCand('jet').pt():-1", float, doc = "pt of associated jet"),
        nStations = Var("numberOfMatchedStations", int, doc = "number of matched stations with default arbitration (segment & track)"), 
        jet = Var("?hasUserCand('jet')?userCand('jet').key():-1", int, doc="index of the associated jet (-1 if none)"),
        mediumId = Var("isPFMuon && innerTrack.validFraction >= 0.49 && ( isGlobalMuon && globalTrack.normalizedChi2 < 3 && combinedQuality.chi2LocalPosition < 12 && combinedQuality.trkKink < 20 && segmentCompatibility >= 0.303 || segmentCompatibility >= 0.451 )", bool, doc = "POG Medium muon ID (2016 tune)"),
    ),
)

metTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = cms.InputTag("slimmedMETs"),
    name = cms.string("MET"),
    singleton = cms.bool(True),
    vars = cms.PSet(PTVars),
)


tables = cms.Sequence(finalMuons + finalJets + linkedObjects + jetTable + muonTable + metTable)

