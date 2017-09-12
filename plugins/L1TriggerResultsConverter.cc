// -*- C++ -*-
//
// Package:    PhysicsTools/NanoAOD
// Class:      L1TriggerResultsConverter
// 
/**\class L1TriggerResultsConverter L1TriggerResultsConverter.cc PhysicsTools/L1TriggerResultsConverter/plugins/L1TriggerResultsConverter.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Andrea Rizzi
//         Created:  Mon, 11 Aug 2017 11:20:30 GMT
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "CondFormats/DataRecord/interface/L1GtTriggerMenuRcd.h"
#include "CondFormats/DataRecord/interface/L1GtTriggerMaskAlgoTrigRcd.h"
#include "CondFormats/L1TObjects/interface/L1GtTriggerMenu.h"
#include "CondFormats/L1TObjects/interface/L1GtTriggerMask.h"
#include "CondFormats/L1TObjects/interface/L1TUtmTriggerMenu.h"
#include "CondFormats/DataRecord/interface/L1TUtmTriggerMenuRcd.h"

#include "DataFormats/L1TGlobal/interface/GlobalAlgBlk.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"

#include "FWCore/Common/interface/TriggerNames.h"
#include "DataFormats/Common/interface/TriggerResults.h"

#include "L1Trigger/GlobalTriggerAnalyzer/interface/L1GtUtils.h"
//
// class declaration
//

class L1TriggerResultsConverter : public edm::stream::EDProducer<> {
   public:
      explicit L1TriggerResultsConverter(const edm::ParameterSet&);
      ~L1TriggerResultsConverter();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

   private:
      virtual void beginStream(edm::StreamID) override;
      virtual void produce(edm::Event&, const edm::EventSetup&) override;
      virtual void endStream() override;

      virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
      //virtual void endRun(edm::Run const&, edm::EventSetup const&) override;
      //virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
      //virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;

      // ----------member data ---------------------------
      const edm::EDGetTokenT<L1GlobalTriggerReadoutRecord> tokenLegacy_;
      const edm::EDGetTokenT<GlobalAlgBlkBxCollection> token_;
      const bool legacyL1_;
      std::vector<std::string> names_;
      std::vector<unsigned int> mask_;
      std::vector<unsigned int> indices_;
};



//
// constructors and destructor
//
L1TriggerResultsConverter::L1TriggerResultsConverter(const edm::ParameterSet& params):
 tokenLegacy_(consumes<L1GlobalTriggerReadoutRecord>( params.getParameter<edm::InputTag>("src") )),
 token_(consumes<GlobalAlgBlkBxCollection>( params.getParameter<edm::InputTag>("src") )),
 legacyL1_( params.getParameter<bool>("legacyL1") )
{
   produces<edm::TriggerResults>();
}


L1TriggerResultsConverter::~L1TriggerResultsConverter()
{
 
   // do anything here that needs to be done at destruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

void L1TriggerResultsConverter::beginRun(edm::Run const&, edm::EventSetup const&setup) {
    mask_.clear();
    names_.clear();
    indices_.clear();
    if(legacyL1_){
        edm::ESHandle<L1GtTriggerMenu> handleMenu;
        edm::ESHandle<L1GtTriggerMask> handleAlgoMask;
        setup.get<L1GtTriggerMenuRcd>().get(handleMenu);
        auto const & mapping = handleMenu->gtAlgorithmAliasMap();
        for (auto const & keyval: mapping) {
	   names_.push_back(keyval.first);
	   indices_.push_back(keyval.second.algoBitNumber()); 
        } 
        setup.get<L1GtTriggerMaskAlgoTrigRcd>().get(handleAlgoMask);
        mask_ = handleAlgoMask->gtTriggerMask();
    } else {
        edm::ESHandle<L1TUtmTriggerMenu> menu;
        setup.get<L1TUtmTriggerMenuRcd>().get(menu);
        auto const & mapping = menu->getAlgorithmMap();
        for (auto const & keyval: mapping) {
           names_.push_back(keyval.first);
	   indices_.push_back(keyval.second.getIndex()); 
        }

    }
}

// ------------ method called to produce the data  ------------


void
L1TriggerResultsConverter::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
    using namespace edm;
    const std::vector<bool> * wordp=nullptr;
    if (!legacyL1_){
      edm::Handle<GlobalAlgBlkBxCollection> handleResults;
       iEvent.getByToken(token_, handleResults);
       wordp= & handleResults->at(0,0).getAlgoDecisionFinal() ;
     } else {
// Legacy access
       edm::Handle<L1GlobalTriggerReadoutRecord> handleResults;
       iEvent.getByToken(tokenLegacy_, handleResults);
       wordp = & handleResults->decisionWord();
    }
    auto const &word = *wordp;
    HLTGlobalStatus l1bitsAsHLTStatus(names_.size());
//    std::cout << word.size() << " " << names_.size() << " " << mask_.size()  << std::endl;
    for(size_t nidx=0;nidx<indices_.size(); nidx++) {
        unsigned int index = indices_[nidx];
        bool result =word[index];
	if(!mask_.empty()) result &=  (mask_[index] !=0);
	l1bitsAsHLTStatus[nidx]=HLTPathStatus(result?edm::hlt::Pass:edm::hlt::Fail);
    }
    //mimic HLT trigger bits for L1
    auto out = std::make_unique<edm::TriggerResults>(l1bitsAsHLTStatus,names_);
    iEvent.put(std::move(out));

}

// ------------ method called once each stream before processing any runs, lumis or events  ------------
void
L1TriggerResultsConverter::beginStream(edm::StreamID)
{
}

// ------------ method called once each stream after processing all runs, lumis and events  ------------
void
L1TriggerResultsConverter::endStream() {
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
L1TriggerResultsConverter::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(L1TriggerResultsConverter);
