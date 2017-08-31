import FWCore.ParameterSet.Config as cms
from  PhysicsTools.NanoAOD.common_cff import *



##################### User floats producers, selectors ##########################



##################### Tables for final output and docs ##########################
pvTable = cms.EDProducer("VertexTableProducer",
    pvSrc = cms.InputTag("offlineSlimmedPrimaryVertices"),
    svSrc = cms.InputTag("slimmedSecondaryVertices"),
    cut = cms.string(""), 
    pvName = cms.string("PV"),
    svName = cms.string("SV"),
    svDoc  = cms.string("secondary vertices from IVF algorithm"),
)

#before cross linking
vertexSequence = cms.Sequence()
#after cross linkining
vertexTables = cms.Sequence( pvTable)

