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



