// -*- C++ -*-
//
// Package:    PhysicsTools/NanoAOD
// Class:      NanoAODBaseCrossCleaner
// 
/**\class NanoAODBaseCrossCleaner NanoAODBaseCrossCleaner.cc PhysicsTools/NanoAODBaseCrossCleaner/plugins/NanoAODBaseCrossCleaner.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Andrea Rizzi
//         Created:  Mon, 28 Aug 2017 09:26:39 GMT
//
//

#include "PhysicsTools/NanoAOD/plugins/NanoAODBaseCrossCleaner.h"
#include "PhysicsTools/NanoAOD/interface/FlatTable.h"

//
// constructors and destructor
//
NanoAODBaseCrossCleaner::NanoAODBaseCrossCleaner(const edm::ParameterSet& params):
    name_(params.getParameter<std::string>("name") ),
    doc_(params.getParameter<std::string>("doc") ),
    jets_(consumes<edm::View<pat::Jet>>( params.getParameter<edm::InputTag>("jets") )),
    muons_(consumes<edm::View<pat::Muon>>( params.getParameter<edm::InputTag>("muons") )),
/*  electrons_(consumes<edm::View<pat::Electron>>( params.getParameter<edm::InputTag>("electrons") )),
    taus_(consumes<edm::View<pat::Tau>>( params.getParameter<edm::InputTag>("taus") )),
    photons_(consumes<edm::View<pat::Photon>>( params.getParameter<edm::InputTag>("photons") ))*/
    jetSel_(params.getParameter<std::string>("jetSel") ),
    muonSel_(params.getParameter<std::string>("muonSel") )

{
   produces<FlatTable>("jets");
   produces<FlatTable>("muons");
/* produces<std::vector<uint8_t>>("electrons");
   produces<std::vector<uint8_t>>("taus");
   produces<std::vector<uint8_t>>("photons");*/
  
}


NanoAODBaseCrossCleaner::~NanoAODBaseCrossCleaner()
{
 
   // do anything here that needs to be done at destruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called to produce the data  ------------


void
NanoAODBaseCrossCleaner::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
    using namespace edm;
    edm::Handle<edm::View<pat::Jet>> jetsIn;
    iEvent.getByToken(jets_, jetsIn);
    std::vector<uint8_t> jets;
    for (const auto & j: *jetsIn) {jets.push_back(jetSel_(j));}
    auto jetsTable = std::make_unique<FlatTable>(jetsIn->size(),"Jet",false);

    edm::Handle<edm::View<pat::Muon>> muonsIn;
    iEvent.getByToken(muons_, muonsIn);
    std::vector<uint8_t> muons;
    for (const auto & m: *muonsIn) {muons.push_back(muonSel_(m));}
    auto muonsTable = std::make_unique<FlatTable>(muonsIn->size(),"Muon",false);

/*
    edm::Handle<edm::View<pat::Electron>> electronsIn;
    iEvent.getByToken(electrons_, electronsIn);
    auto electrons = std::make_unique<std::vector<pat::Electron>>();

    edm::Handle<edm::View<pat::Tau>> tausIn;
    iEvent.getByToken(taus_, tausIn);
    auto taus = std::make_unique<std::vector<pat::Tau>>();

    edm::Handle<edm::View<pat::Photon>> photonsIn;
    iEvent.getByToken(photons_, photonsIn);
    auto photons = std::make_unique<std::vector<pat::Photon>>();

*/
    objectSelection(*muonsIn,*jetsIn,muons,jets);

    muonsTable->addColumn<uint8_t>(name_,muons,doc_,FlatTable::UInt8Column);
    jetsTable->addColumn<uint8_t>(name_,jets,doc_,FlatTable::UInt8Column);

    iEvent.put(std::move(jetsTable),"jets");
    iEvent.put(std::move(muonsTable),"muons");
/*    iEvent.put(std::move(electrons),"electrons");
    iEvent.put(std::move(taus),"taus");
    iEvent.put(std::move(photons),"photons");*/
 
}

// ------------ method called once each stream before processing any runs, lumis or events  ------------
void
NanoAODBaseCrossCleaner::beginStream(edm::StreamID)
{
}

// ------------ method called once each stream after processing all runs, lumis and events  ------------
void
NanoAODBaseCrossCleaner::endStream() {
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
NanoAODBaseCrossCleaner::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(NanoAODBaseCrossCleaner);
