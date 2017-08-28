// -*- C++ -*-
//
// Package:    PhysicsTools/NanoAOD
// Class:      PATObjectCrossLinker
// 
/**\class PATObjectCrossLinker PATObjectCrossLinker.cc PhysicsTools/PATObjectCrossLinker/plugins/PATObjectCrossLinker.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Andrea Rizzi
//         Created:  Mon, 28 Aug 2017 09:26:39 GMT
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"

#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/Tau.h"

#include "PhysicsTools/NanoAOD/interface/MatchingUtils.h"
//
// class declaration
//

class PATObjectCrossLinker : public edm::stream::EDProducer<> {
   public:
      explicit PATObjectCrossLinker(const edm::ParameterSet&);
      ~PATObjectCrossLinker();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

   private:
      virtual void beginStream(edm::StreamID) override;
      virtual void produce(edm::Event&, const edm::EventSetup&) override;
      virtual void endStream() override;

      void matchOneToMany(const auto & refProdOne, auto & itemsOne, const std::string & nameOne,
                    const auto & refProdMany, auto& itemsMany, const std::string & nameMany);

      //virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
      //virtual void endRun(edm::Run const&, edm::EventSetup const&) override;
      //virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
      //virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;

      // ----------member data ---------------------------
      const edm::EDGetTokenT<std::vector<pat::Jet>> jets_;
      const edm::EDGetTokenT<edm::View<reco::Candidate>> muons_;
      const edm::EDGetTokenT<std::vector<pat::Electron>> electrons_;
      const edm::EDGetTokenT<std::vector<pat::Tau>> taus_;
      const edm::EDGetTokenT<std::vector<pat::Photon>> photons_;


};

//
// constructors and destructor
//
PATObjectCrossLinker::PATObjectCrossLinker(const edm::ParameterSet& params):
    jets_(consumes<std::vector<pat::Jet>>( params.getParameter<edm::InputTag>("jets") )),
    muons_(consumes<edm::View<reco::Candidate>>( params.getParameter<edm::InputTag>("muons") )),
    electrons_(consumes<std::vector<pat::Electron>>( params.getParameter<edm::InputTag>("electrons") )),
    taus_(consumes<std::vector<pat::Tau>>( params.getParameter<edm::InputTag>("taus") )),
    photons_(consumes<std::vector<pat::Photon>>( params.getParameter<edm::InputTag>("photons") ))

{
   produces<std::vector<pat::Jet>>("jets");
   produces<std::vector<pat::Muon>>("muons");
   produces<std::vector<pat::Electron>>("electrons");
   produces<std::vector<pat::Tau>>("taus");
   produces<std::vector<pat::Photon>>("photons");
  
}


PATObjectCrossLinker::~PATObjectCrossLinker()
{
 
   // do anything here that needs to be done at destruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called to produce the data  ------------

///
void PATObjectCrossLinker::matchOneToMany(const auto & refProdOne, auto & itemsOne, const std::string & nameOne,
		    const auto & refProdMany, auto& itemsMany, const std::string & nameMany)
{
    size_t ji=0;
    for(auto & j: itemsOne) {
        edm::PtrVector<reco::Candidate> overlaps(refProdMany.id());
        size_t mi=0;
        for(auto & m: itemsMany){
            if(matchByCommonSourceCandidatePtr(j,m)){
                m.addUserCand(nameOne,reco::CandidatePtr(refProdOne.id(), ji, refProdOne.productGetter()));
                overlaps.push_back(reco::CandidatePtr(refProdMany.id(), mi, refProdMany.productGetter()));
            }
            mi++;
        }
    j.setOverlaps(nameMany,overlaps);
    ji++;
   } 
}


void
PATObjectCrossLinker::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
    using namespace edm;
    edm::Handle<std::vector<pat::Jet>> jetsIn;
    iEvent.getByToken(jets_, jetsIn);
    auto jets = std::make_unique<std::vector<pat::Jet>>(*jetsIn);
    auto jetRefProd =  iEvent.getRefBeforePut< std::vector<pat::Jet> >("jets");

    edm::Handle<edm::View<reco::Candidate>> muonsIn;
    iEvent.getByToken(muons_, muonsIn);
    auto muons = std::make_unique<std::vector<pat::Muon>>();
    for(const auto & m  : *muonsIn) {muons->push_back(pat::Muon(*dynamic_cast<const pat::Muon *>(&m)));}
    auto muRefProd =  iEvent.getRefBeforePut< std::vector<pat::Muon> >("muons");

    edm::Handle<std::vector<pat::Electron>> electronsIn;
    iEvent.getByToken(electrons_, electronsIn);
    auto electrons = std::make_unique<std::vector<pat::Electron>>(*electronsIn);
    auto eleRefProd =  iEvent.getRefBeforePut< std::vector<pat::Electron> >("electrons");

    edm::Handle<std::vector<pat::Tau>> tausIn;
    iEvent.getByToken(taus_, tausIn);
    auto taus = std::make_unique<std::vector<pat::Tau>>(*tausIn);
    auto tauRefProd =  iEvent.getRefBeforePut< std::vector<pat::Tau> >("taus");

    edm::Handle<std::vector<pat::Photon>> photonsIn;
    iEvent.getByToken(photons_, photonsIn);
    auto photons = std::make_unique<std::vector<pat::Photon>>(*photonsIn);
    auto phRefProd =  iEvent.getRefBeforePut< std::vector<pat::Photon> >("photons");

    matchOneToMany(jetRefProd,*jets,"jet",muRefProd,*muons,"muons");
    matchOneToMany(jetRefProd,*jets,"jet",eleRefProd,*electrons,"electrons");
    matchOneToMany(jetRefProd,*jets,"jet",tauRefProd,*taus,"taus");
    matchOneToMany(jetRefProd,*jets,"jet",phRefProd,*photons,"photons");


    iEvent.put(std::move(jets),"jets");
    iEvent.put(std::move(muons),"muons");
    iEvent.put(std::move(electrons),"electrons");
    iEvent.put(std::move(taus),"taus");
    iEvent.put(std::move(photons),"photons");
 
}

// ------------ method called once each stream before processing any runs, lumis or events  ------------
void
PATObjectCrossLinker::beginStream(edm::StreamID)
{
}

// ------------ method called once each stream after processing all runs, lumis and events  ------------
void
PATObjectCrossLinker::endStream() {
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
PATObjectCrossLinker::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(PATObjectCrossLinker);
