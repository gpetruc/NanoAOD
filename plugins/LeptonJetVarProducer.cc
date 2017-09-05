// -*- C++ -*-
//
// Package:    PhysicsTools/NanoAOD
// Class:      LeptonJetVarProducer
// 
/**\class LeptonJetVarProducer LeptonJetVarProducer.cc PhysicsTools/NanoAOD/plugins/LeptonJetVarProducer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Marco Peruzzi
//         Created:  Tue, 05 Sep 2017 12:24:38 GMT
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

#include "TLorentzVector.h"
#include "DataFormats/Common/interface/View.h"

#include "PhysicsTools/NanoAOD/interface/MatchingUtils.h"

//
// class declaration
//

template <typename T>
class LeptonJetVarProducer : public edm::stream::EDProducer<> {
   public:
  explicit LeptonJetVarProducer(const edm::ParameterSet &iConfig):
    srcJet_(consumes<edm::View<pat::Jet>>(iConfig.getParameter<edm::InputTag>("srcJet"))),
    srcLep_(consumes<edm::View<T>>(iConfig.getParameter<edm::InputTag>("srcLep")))
  {
    produces<edm::ValueMap<float>>("ptRatio");
    produces<edm::ValueMap<float>>("ptRel");
    produces<edm::ValueMap<reco::CandidatePtr>>("jetForLepJetVar");
  }
  ~LeptonJetVarProducer() {};

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

   private:
  virtual void beginStream(edm::StreamID) override {};
  virtual void produce(edm::Event&, const edm::EventSetup&) override;
  virtual void endStream() override {};

      //virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
      //virtual void endRun(edm::Run const&, edm::EventSetup const&) override;
      //virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
      //virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;

  std::pair<float,float> calculatePtRatioRel(auto &lep, auto &jet);

      // ----------member data ---------------------------

  edm::EDGetTokenT<edm::View<pat::Jet>> srcJet_;
  edm::EDGetTokenT<edm::View<T>> srcLep_;
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
LeptonJetVarProducer<T>::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{

  edm::Handle<edm::View<pat::Jet>> srcJet;
  iEvent.getByToken(srcJet_, srcJet);
  edm::Handle<edm::View<T>> srcLep;
  iEvent.getByToken(srcLep_, srcLep);

  std::vector<float> ptRatio(srcLep->size(),-1);
  std::vector<float> ptRel(srcLep->size(),-1);
  std::vector<reco::CandidatePtr> jetForLepJetVar(srcLep->size(),reco::CandidatePtr());

  for (uint il = 0; il<srcLep->size(); il++){
    for (uint ij = 0; ij<srcJet->size(); ij++){
      auto lep = srcLep->ptrAt(il);
      auto jet = srcJet->ptrAt(ij);
      if(matchByCommonSourceCandidatePtr(*lep,*jet)){
	  auto res = calculatePtRatioRel(lep,jet);
	  ptRatio[il] = res.first;
	  ptRel[il] = res.second;
	  jetForLepJetVar[il] = jet;
	  break; // take leading jet with shared source candidates
	}
    }
  }

  std::unique_ptr<edm::ValueMap<float>> ptRatioV(new edm::ValueMap<float>());
  edm::ValueMap<float>::Filler fillerRatio(*ptRatioV);
  fillerRatio.insert(srcLep,ptRatio.begin(),ptRatio.end());
  fillerRatio.fill();
  iEvent.put(std::move(ptRatioV),"ptRatio");

  std::unique_ptr<edm::ValueMap<float>> ptRelV(new edm::ValueMap<float>());
  edm::ValueMap<float>::Filler fillerRel(*ptRelV);
  fillerRel.insert(srcLep,ptRel.begin(),ptRel.end());
  fillerRel.fill();
  iEvent.put(std::move(ptRelV),"ptRel");

  std::unique_ptr<edm::ValueMap<reco::CandidatePtr>> jetForLepJetVarV(new edm::ValueMap<reco::CandidatePtr>());
  edm::ValueMap<reco::CandidatePtr>::Filler fillerjetForLepJetVar(*jetForLepJetVarV);
  fillerjetForLepJetVar.insert(srcLep,jetForLepJetVar.begin(),jetForLepJetVar.end());
  fillerjetForLepJetVar.fill();
  iEvent.put(std::move(jetForLepJetVarV),"jetForLepJetVar");


}

template <typename T>
std::pair<float,float>
LeptonJetVarProducer<T>::calculatePtRatioRel(auto &lep, auto &jet) {
 
  auto rawp4_ = jet->correctedP4("Uncorrected");
  auto rawp4 = TLorentzVector(rawp4_.pt(),rawp4_.eta(),rawp4_.phi(),rawp4_.energy());
  auto lepp4 = TLorentzVector(lep->pt(),lep->eta(),lep->phi(),lep->energy());

  if ((rawp4-lepp4).Rho()<1e-4) return std::make_pair<float,float>(1.0,0.0);

  auto jetp4 = (rawp4 - lepp4*(1.0/jet->jecFactor("L1FastJet")))*(jet->pt()/rawp4.Pt())+lepp4;
  auto ptratio = lepp4.Pt()/jetp4.Pt();
  auto ptrel = lepp4.Perp((jetp4-lepp4).Vect());

  return std::make_pair<float,float>(ptratio,ptrel);
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
template <typename T>
void
LeptonJetVarProducer<T>::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

typedef LeptonJetVarProducer<pat::Muon> MuonJetVarProducer;
typedef LeptonJetVarProducer<pat::Electron> ElectronJetVarProducer;

//define this as a plug-in
DEFINE_FWK_MODULE(MuonJetVarProducer);
DEFINE_FWK_MODULE(ElectronJetVarProducer);
