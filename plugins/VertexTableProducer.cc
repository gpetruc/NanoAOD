// -*- C++ -*-
//
// Package:    PhysicsTools/NanoAOD
// Class:      VertexTableProducer
// 
/**\class VertexTableProducer VertexTableProducer.cc PhysicsTools/VertexTableProducer/plugins/VertexTableProducer.cc

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

#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/Candidate/interface/VertexCompositePtrCandidate.h"

//#include "CommonTools/Utils/interface/StringCutObjectSelector.h"

#include "PhysicsTools/NanoAOD/interface/FlatTable.h"
//
// class declaration
//

class VertexTableProducer : public edm::stream::EDProducer<> {
   public:
      explicit VertexTableProducer(const edm::ParameterSet&);
      ~VertexTableProducer();

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

      const edm::EDGetTokenT<std::vector<reco::Vertex>> pvs_;
      const edm::EDGetTokenT<std::vector<reco::VertexCompositePtrCandidate> > svs_;
      const std::string  pvName_;
      const std::string  svName_;
      const std::string svDoc_;


};



//
// constructors and destructor
//
VertexTableProducer::VertexTableProducer(const edm::ParameterSet& params):
    pvs_(consumes<std::vector<reco::Vertex>>( params.getParameter<edm::InputTag>("pvSrc") )),
    svs_(consumes<std::vector<reco::VertexCompositePtrCandidate> >( params.getParameter<edm::InputTag>("svSrc") )),
    pvName_(params.getParameter<std::string>("pvName") ),
    svName_(params.getParameter<std::string>("svName") ),
    svDoc_(params.getParameter<std::string>("svDoc") )
   
{
   produces<FlatTable>("pvs");
   produces<FlatTable>("svs");
  
}


VertexTableProducer::~VertexTableProducer()
{
 
   // do anything here that needs to be done at destruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called to produce the data  ------------


void
VertexTableProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
    using namespace edm;
    edm::Handle<std::vector<reco::Vertex>> pvsIn;
    iEvent.getByToken(pvs_, pvsIn);
    auto pvsTable = std::make_unique<FlatTable>(1,pvName_,true);

    edm::Handle<std::vector<reco::VertexCompositePtrCandidate> > svsIn;
    iEvent.getByToken(svs_, svsIn);
    auto svsTable = std::make_unique<FlatTable>(svsIn->size(),svName_,false);

    pvsTable->addColumnValue<float>("ndof",(*pvsIn)[0].ndof(),"",FlatTable::FloatColumn);
    pvsTable->addColumnValue<float>("x",(*pvsIn)[0].position().x(),"vertex position x coordinate",FlatTable::FloatColumn);
    pvsTable->addColumnValue<float>("y",(*pvsIn)[0].position().y(),"vertex position y coordinate",FlatTable::FloatColumn);
    pvsTable->addColumnValue<float>("z",(*pvsIn)[0].position().z(),"vertex position z coordinate",FlatTable::FloatColumn);
    pvsTable->addColumnValue<float>("chi2",(*pvsIn)[0].normalizedChi2(),"reduced chi2",FlatTable::FloatColumn);

    
/*  for(const auto & sv :  *svsIn){
    }	
    svsTable->addColumn<uint8_t>(name_,muons,doc_,FlatTable::UInt8Column);*/
 

    iEvent.put(std::move(pvsTable),"pvs");
    iEvent.put(std::move(svsTable),"svs");
 
}

// ------------ method called once each stream before processing any runs, lumis or events  ------------
void
VertexTableProducer::beginStream(edm::StreamID)
{
}

// ------------ method called once each stream after processing all runs, lumis and events  ------------
void
VertexTableProducer::endStream() {
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
VertexTableProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(VertexTableProducer);
