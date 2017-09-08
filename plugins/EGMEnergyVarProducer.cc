// -*- C++ -*-
//
// Package:    PhysicsTools/NanoAOD
// Class:      EGMEnergyVarProducer
// 
/**\class EGMEnergyVarProducer EGMEnergyVarProducer.cc PhysicsTools/NanoAOD/plugins/EGMEnergyVarProducer.cc
 Description: [one line class summary]
 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Emanuele Di Marco
//         Created:  Wed, 06 Sep 2017 12:34:38 GMT
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

#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Photon.h"

#include "TLorentzVector.h"
#include "DataFormats/Common/interface/View.h"

#include "PhysicsTools/NanoAOD/interface/MatchingUtils.h"

//
// class declaration
//

template <typename T>
class EGMEnergyVarProducer : public edm::stream::EDProducer<> {
public:
  explicit EGMEnergyVarProducer(const edm::ParameterSet &iConfig):
    srcRaw_(consumes<edm::View<T>>(iConfig.getParameter<edm::InputTag>("srcRaw"))),
    srcCorr_(consumes<edm::View<T>>(iConfig.getParameter<edm::InputTag>("srcCorr")))
  {
    produces<edm::ValueMap<float>>("eCorr");
  }
    ~EGMEnergyVarProducer() {};

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  virtual void beginStream(edm::StreamID) override {};
  virtual void produce(edm::Event&, const edm::EventSetup&) override;
  virtual void endStream() override {};

  //virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
  //virtual void endRun(edm::Run const&, edm::EventSetup const&) override;
  //virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
  //virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;

  // ----------member data ---------------------------

  edm::EDGetTokenT<edm::View<T>> srcRaw_;
  edm::EDGetTokenT<edm::View<T>> srcCorr_;
};

//
// constants, enums and typedefs
//


//
// static data member definitions
//

//
// member functions
//

// ------------ method called to produce the data  ------------
template <typename T>
void
EGMEnergyVarProducer<T>::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  edm::Handle<edm::View<T>> srcRaw;
  iEvent.getByToken(srcRaw_, srcRaw);
  edm::Handle<edm::View<T>> srcCorr;
  iEvent.getByToken(srcCorr_, srcCorr);

  std::vector<float> eCorr(srcCorr->size(),-1);

  for (uint ir = 0; ir<srcRaw->size(); ir++){
    for (uint ic = 0; ic<srcCorr->size(); ic++){
      auto egm_raw = srcRaw->ptrAt(ir);
      auto egm_corr = srcCorr->ptrAt(ic);
      if(matchByCommonParentSuperClusterRef(*egm_raw,*egm_corr)){
          eCorr[ir] = egm_corr->energy()/egm_raw->energy();
          break;
      }
    }
  }

  std::unique_ptr<edm::ValueMap<float>> eCorrV(new edm::ValueMap<float>());
  edm::ValueMap<float>::Filler fillerCorr(*eCorrV);
  fillerCorr.insert(srcRaw,eCorr.begin(),eCorr.end());
  fillerCorr.fill();
  iEvent.put(std::move(eCorrV),"eCorr");

}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
template <typename T>
void
EGMEnergyVarProducer<T>::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

typedef EGMEnergyVarProducer<pat::Electron> ElectronEnergyVarProducer;
typedef EGMEnergyVarProducer<pat::Photon> PhotonEnergyVarProducer;

//define this as a plug-in
DEFINE_FWK_MODULE(ElectronEnergyVarProducer);
DEFINE_FWK_MODULE(PhotonEnergyVarProducer);
