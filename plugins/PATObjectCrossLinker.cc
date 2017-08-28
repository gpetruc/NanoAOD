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
      
      //virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
      //virtual void endRun(edm::Run const&, edm::EventSetup const&) override;
      //virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
      //virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;

      // ----------member data ---------------------------
      const edm::EDGetTokenT<std::vector<pat::Jet>> jets_;
      const edm::EDGetTokenT<std::vector<pat::Muon>> muons_;

};

//
// constructors and destructor
//
PATObjectCrossLinker::PATObjectCrossLinker(const edm::ParameterSet& params):
    jets_(consumes<std::vector<pat::Jet>>( params.getParameter<edm::InputTag>("jets") )),
    muons_(consumes<std::vector<pat::Muon>>( params.getParameter<edm::InputTag>("muons") ))

{
   produces<std::vector<pat::Jet>>("jets");
   produces<std::vector<pat::Muon>>("muons");
  
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
void
PATObjectCrossLinker::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
    using namespace edm;
    edm::Handle<std::vector<pat::Jet>> jetsIn;
    iEvent.getByToken(jets_, jetsIn);
    edm::Handle<std::vector<pat::Muon>> muonsIn;
    iEvent.getByToken(muons_, muonsIn);

    auto jets = std::make_unique<std::vector<pat::Jet>>(*jetsIn);
    auto muons = std::make_unique<std::vector<pat::Muon>>(*muonsIn);

    auto jetRefProd =  iEvent.getRefBeforePut< std::vector<pat::Jet> >("jets");
    for(auto & m: *muons) {
	int key=-1;
	size_t ji=0;
	for(auto & j: *jets){
	    for(size_t i1 = 0 ; i1 < j.numberOfSourceCandidatePtrs();i1++){
	       auto  c1=j.sourceCandidatePtr(i1);
	       for(size_t i2 = 0 ; i2 < m.numberOfSourceCandidatePtrs();i2++) {
	          if(j.sourceCandidatePtr(i2)==c1) key=ji;
	       }
		
	    }
	    ji++;
        }
	if(key>0) m.addUserCand("jet",edm::Ptr<pat::Jet>(jetRefProd.id(), key, jetRefProd.productGetter())); 
    }	

    iEvent.put(std::move(jets),"jets");
    iEvent.put(std::move(muons),"muons");
 
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
