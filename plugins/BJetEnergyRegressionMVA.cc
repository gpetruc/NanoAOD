//
// Original Author:  Andre Rizzi
//         Created:  Mon, 07 Sep 2017 09:18:03 GMT
//
//

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"


#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Jet.h"

#include "DataFormats/VertexReco/interface/Vertex.h"

#include "PhysicsTools/NanoAOD/plugins/BaseMVATemplateProducer.h"
#include <vector>

template <typename O>
class BJetEnergyRegressionMVA : public BaseMVATemplateProducer<pat::Jet,O> {
	public:
	  explicit BJetEnergyRegressionMVA(const edm::ParameterSet &iConfig):
		BaseMVATemplateProducer<pat::Jet,O>(iConfig),
    		pvsrc_(edm::stream::EDProducer<>::consumes<std::vector<reco::Vertex>>(iConfig.getParameter<edm::InputTag>("pvsrc")))
	  {

/*      Jet_leptonPtRel = cms.string("?overlaps('muons').size()>0?overlaps('muons')[0].userFloat('ptRel'):(?overlaps('electrons').size()>0?overlaps('electrons')[0].userFloat('ptRel'):-1)"),
        Jet_leadTrackPt = cms.string("pt"),
        Jet_vtxPt = cms.string("pt"),
        Jet_vtxMass = cms.string("pt"),
        Jet_vtx3dL = cms.string("pt"),
        Jet_vtxNtrk = cms.string("pt"),
        Jet_vtx3deL = cms.string("pt"),*/
		
	  }
	  virtual void readAdditionalCollections(edm::Event&iEvent, const edm::EventSetup&) override {
		iEvent.getByToken(pvsrc_, pvs_);
	  }

          virtual void fillAdditionalVariables(const pat::Jet&j)  override {
		BaseMVATemplateProducer<pat::Jet,O>::setValue("nPVs",pvs_->size());
		BaseMVATemplateProducer<pat::Jet,O>::setValue("Jet_leptonPtRel",0);

		if(j.overlaps("muons").size() >0) { 
			const auto *lep=dynamic_cast<const pat::Muon *>(&*j.overlaps("muons")[0]);
			if(lep!=nullptr) {BaseMVATemplateProducer<pat::Jet,O>::setValue("Jet_leptonPtRel",lep->userFloat("ptRel"));}
		}
		else if(j.overlaps("electrons").size() >0) {
			const auto *lep=dynamic_cast<const pat::Electron *>(&*j.overlaps("electrons")[0]);
			if(lep!=nullptr) {BaseMVATemplateProducer<pat::Jet,O>::setValue("Jet_leptonPtRel",lep->userFloat("ptRel"));}
		}
		
		float ptMax=0;
		for(const auto & d : j.daughterPtrVector()){if(d->pt()>ptMax) ptMax=d->pt();}
		BaseMVATemplateProducer<pat::Jet,O>::setValue("Jet_leadTrackPt",ptMax);

	  }
        private:
	  edm::EDGetTokenT<std::vector<reco::Vertex>> pvsrc_;
 	  edm::Handle<std::vector<reco::Vertex>> pvs_;
	  
};

//define this as a plug-in
typedef BJetEnergyRegressionMVA<edm::ValueMap<float>> BJetEnergyRegressionMVAValueMap;
typedef BJetEnergyRegressionMVA<FlatTable> BJetEnergyRegressionMVATable;
DEFINE_FWK_MODULE(BJetEnergyRegressionMVAValueMap);
DEFINE_FWK_MODULE(BJetEnergyRegressionMVATable);

