import FWCore.ParameterSet.Config as cms
from  PhysicsTools.NanoAOD.common_cff import *



##################### User floats producers, selectors ##########################



##################### Tables for final output and docs ##########################
vertexTable = cms.EDProducer("VertexTableProducer",
    pvSrc = cms.InputTag("offlineSlimmedPrimaryVertices"),
    svSrc = cms.InputTag("slimmedSecondaryVertices"),
#    cut = cms.string(""),  #not implemented
    pvName = cms.string("PV"),
    svName = cms.string("SV"),
    svDoc  = cms.string("secondary vertices from IVF algorithm"),
)

svCandidateTable =  cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = cms.InputTag("slimmedSecondaryVertices"),
    cut = cms.string(""), #we should not filter unless we filter also on the vertexTable
    name = cms.string("SV"),
    singleton = cms.bool(False), # the number of entries is variable
    extension = cms.bool(True), 
    variables = cms.PSet(P4Vars,
        x   = Var("position().x()", float, doc = "secondary vertex X position, in cm",precision=10),
        y   = Var("position().y()", float, doc = "secondary vertex Y position, in cm",precision=10),
        z   = Var("position().z()", float, doc = "secondary vertex Z position, in cm",precision=14),
        ndof   = Var("vertexNdof()", float, doc = "number of degrees of freedom",precision=8),
        chi2   = Var("vertexNormalizedChi2()", float, doc = "reduced chi2, i.e. chi/ndof",precision=8),
    ),
)

#before cross linking
vertexSequence = cms.Sequence()
#after cross linkining
vertexTables = cms.Sequence( vertexTable+svCandidateTable)

