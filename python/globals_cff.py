import FWCore.ParameterSet.Config as cms
from PhysicsTools.NanoAOD.common_cff import OVar

def ExtVar(tag, valtype, compression=None, doc=None, mcOnly=False):
    """Create a PSet for a variable read from the event

       tag is the InputTag to the variable. 

       see OVar in common_cff for all the other arguments
    """
    return OVar(valtype, compression=compression, doc=(doc if doc else tag.encode()), mcOnly=mcOnly).clone(
                src = tag if type(tag) == cms.InputTag else cms.InputTag(tag),
          )

rhoTable = cms.EDProducer("GlobalVariablesTableProducer",
    variables = cms.PSet(
        fixedGridRhoFastjetAll = ExtVar( cms.InputTag("fixedGridRhoFastjetAll"), "double", doc = "rho from all PF Candidates, used e.g. for JECs" ),
        fixedGridRhoFastjetCentralNeutral = ExtVar( cms.InputTag("fixedGridRhoFastjetCentralNeutral"), "double", doc = "rho from neutral PF Candidates with |eta| < 2.5, used e.g. for rho corrections of some lepton isolations" ),
        fixedGridRhoFastjetCentralCalo = ExtVar( cms.InputTag("fixedGridRhoFastjetCentralCalo"), "double", doc = "rho from calo towers with |eta| < 2.5, used e.g. egamma PFCluster isolation" ),
    )
)

globalTables = cms.Sequence(rhoTable)

