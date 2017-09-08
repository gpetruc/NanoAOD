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
#include "DataFormats/Candidate/interface/VertexCompositePtrCandidate.h"

#include "RecoVertex/VertexTools/interface/VertexDistance3D.h"
#include "RecoVertex/VertexPrimitives/interface/ConvertToFromReco.h"
#include "RecoVertex/VertexPrimitives/interface/VertexState.h"

#include "PhysicsTools/NanoAOD/plugins/BaseMVATemplateProducer.h"
#include <vector>

template <typename O>
class BJetEnergyRegressionMVA : public BaseMVATemplateProducer<pat::Jet,O> {
	public:
	  explicit BJetEnergyRegressionMVA(const edm::ParameterSet &iConfig):
		BaseMVATemplateProducer<pat::Jet,O>(iConfig),
    		pvsrc_(edm::stream::EDProducer<>::consumes<std::vector<reco::Vertex>>(iConfig.getParameter<edm::InputTag>("pvsrc"))),
    		svsrc_(edm::stream::EDProducer<>::consumes<edm::View<reco::VertexCompositePtrCandidate>> (iConfig.getParameter<edm::InputTag>("svsrc")))

	  {

		
	  }
	  virtual void readAdditionalCollections(edm::Event&iEvent, const edm::EventSetup&) override {
		iEvent.getByToken(pvsrc_, pvs_);
		iEvent.getByToken(svsrc_, svs_);
	  }

          virtual void fillAdditionalVariables(const pat::Jet&j)  override {
		this->setValue("nPVs",pvs_->size());
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

		//Fill vertex properties
		VertexDistance3D vdist;
		float maxFoundSignificance=0;
		const auto & pv = (*pvs_)[0];
	  	this->setValue("Jet_vtxPt",0);
	   	this->setValue("Jet_vtxMass",0);
    		this->setValue("Jet_vtx3dL",0);
    		this->setValue("Jet_vtx3deL",0);
    		this->setValue("Jet_vtxNtrk",0);

		for(const auto &sv: *svs_){
	      	GlobalVector flightDir(sv.vertex().x() - pv.x(), sv.vertex().y() - pv.y(),sv.vertex().z() - pv.z());
	            GlobalVector jetDir(j.px(),j.py(),j.pz());
                    if( Geom::deltaR2( flightDir, jetDir ) < 0.09 ){
		        Measurement1D dl= vdist.distance(pv,VertexState(RecoVertex::convertPos(sv.position()),RecoVertex::convertError(sv.error())));
			if(dl.significance() > maxFoundSignificance){
				 maxFoundSignificance=dl.significance();
				 this->setValue("Jet_vtxPt",sv.pt());
				 this->setValue("Jet_vtxMass",sv.p4().M());
				 this->setValue("Jet_vtx3dL",dl.value());
				 this->setValue("Jet_vtx3deL",dl.error());
				 this->setValue("Jet_vtxNtrk",sv.numberOfSourceCandidatePtrs());
			}	
		    } 
		}

	  }
        private:
	  const edm::EDGetTokenT<std::vector<reco::Vertex>> pvsrc_;
 	  edm::Handle<std::vector<reco::Vertex>> pvs_;
          const edm::EDGetTokenT<edm::View<reco::VertexCompositePtrCandidate> > svsrc_;
 	  edm::Handle<edm::View<reco::VertexCompositePtrCandidate>> svs_;
	  
};

//define this as a plug-in
typedef BJetEnergyRegressionMVA<edm::ValueMap<float>> BJetEnergyRegressionMVAValueMap;
typedef BJetEnergyRegressionMVA<FlatTable> BJetEnergyRegressionMVATable;
DEFINE_FWK_MODULE(BJetEnergyRegressionMVAValueMap);
DEFINE_FWK_MODULE(BJetEnergyRegressionMVATable);

