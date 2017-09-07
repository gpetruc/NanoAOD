// -*- C++ -*-
//
// Package:    PhysicsTools/NanoAOD
// Class:      BaseMVAValueMapProducer
// 
/**\class BaseMVAValueMapProducer BaseMVAValueMapProducer.cc PhysicsTools/NanoAOD/plugins/BaseMVAValueMapProducer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Andre Rizzi
//         Created:  Mon, 07 Sep 2017 09:18:03 GMT
//
//



#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Jet.h"


#include "PhysicsTools/NanoAOD/plugins/BaseMVATemplateProducer.h"

typedef BaseMVATemplateProducer<pat::Jet,edm::ValueMap<float>> JetBaseMVAValueMapProducer;
typedef BaseMVATemplateProducer<pat::Muon,edm::ValueMap<float>> MuonBaseMVAValueMapProducer;
typedef BaseMVATemplateProducer<pat::Electron,edm::ValueMap<float>> EleBaseMVAValueMapProducer;
typedef BaseMVATemplateProducer<pat::Jet,FlatTable> JetBaseMVAFlatTableProducer;
typedef BaseMVATemplateProducer<pat::Muon,FlatTable> MuonBaseMVAFlatTableProducer;
typedef BaseMVATemplateProducer<pat::Electron,FlatTable> EleBaseMVAFlatTableProducer;

//define this as a plug-in
DEFINE_FWK_MODULE(JetBaseMVAValueMapProducer);
DEFINE_FWK_MODULE(MuonBaseMVAValueMapProducer);
DEFINE_FWK_MODULE(EleBaseMVAValueMapProducer);
DEFINE_FWK_MODULE(JetBaseMVAFlatTableProducer);
DEFINE_FWK_MODULE(MuonBaseMVAFlatTableProducer);
DEFINE_FWK_MODULE(EleBaseMVAFlatTableProducer);

