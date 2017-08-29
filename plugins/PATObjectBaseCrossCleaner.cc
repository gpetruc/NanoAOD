// -*- C++ -*-
//
// Package:    PhysicsTools/NanoAOD
// Class:      PATObjectBaseCrossCleaner
// 
/**\class PATObjectBaseCrossCleaner PATObjectBaseCrossCleaner.cc PhysicsTools/PATObjectBaseCrossCleaner/plugins/PATObjectBaseCrossCleaner.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Andrea Rizzi
//         Created:  Mon, 28 Aug 2017 09:26:39 GMT
//
//

#include "PhysicsTools/NanoAOD/plugins/PATObjectBaseCrossCleaner.h"

//
// constructors and destructor
//
PATObjectBaseCrossCleaner::PATObjectBaseCrossCleaner(const edm::ParameterSet& params):
    jets_(consumes<edm::View<pat::Jet>>( params.getParameter<edm::InputTag>("jets") )),
    muons_(consumes<edm::View<pat::Muon>>( params.getParameter<edm::InputTag>("muons") )),
/*  electrons_(consumes<edm::View<pat::Electron>>( params.getParameter<edm::InputTag>("electrons") )),
    taus_(consumes<edm::View<pat::Tau>>( params.getParameter<edm::InputTag>("taus") )),
    photons_(consumes<edm::View<pat::Photon>>( params.getParameter<edm::InputTag>("photons") ))*/
    jetSel_(params.getParameter<std::string>("jetSel") ),
    muonSel_(params.getParameter<std::string>("muonSel") )

{
   produces<std::vector<uint8_t>>("jets");
   produces<std::vector<uint8_t>>("muons");
/* produces<std::vector<uint8_t>>("electrons");
   produces<std::vector<uint8_t>>("taus");
   produces<std::vector<uint8_t>>("photons");*/
  
}


PATObjectBaseCrossCleaner::~PATObjectBaseCrossCleaner()
{
 
   // do anything here that needs to be done at destruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called to produce the data  ------------


void
PATObjectBaseCrossCleaner::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
    using namespace edm;
    edm::Handle<edm::View<pat::Jet>> jetsIn;
    iEvent.getByToken(jets_, jetsIn);
    auto jets = std::make_unique<std::vector<uint8_t>>();
    for (const auto & j: *jetsIn) {jets->push_back(jetSel_(j));}

    edm::Handle<edm::View<pat::Muon>> muonsIn;
    iEvent.getByToken(muons_, muonsIn);
    auto muons = std::make_unique<std::vector<uint8_t>>();
    for (const auto & m: *muonsIn) {muons->push_back(muonSel_(m));}
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
    objectSelection(*muonsIn,*jetsIn,*muons,*jets);

    iEvent.put(std::move(jets),"jets");
    iEvent.put(std::move(muons),"muons");
/*    iEvent.put(std::move(electrons),"electrons");
    iEvent.put(std::move(taus),"taus");
    iEvent.put(std::move(photons),"photons");*/
 
}

// ------------ method called once each stream before processing any runs, lumis or events  ------------
void
PATObjectBaseCrossCleaner::beginStream(edm::StreamID)
{
}

// ------------ method called once each stream after processing all runs, lumis and events  ------------
void
PATObjectBaseCrossCleaner::endStream() {
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
PATObjectBaseCrossCleaner::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(PATObjectBaseCrossCleaner);
