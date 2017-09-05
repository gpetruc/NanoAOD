#include "FWCore/Framework/interface/global/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "PhysicsTools/NanoAOD/interface/FlatTable.h"
#include "DataFormats/Common/interface/View.h"
#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"

#include <vector>
#include <iostream>


class CandMCMatchTableProducer : public edm::global::EDProducer<> {
    public:
        CandMCMatchTableProducer( edm::ParameterSet const & params ) :
            objName_(params.getParameter<std::string>("objName")),
            branchName_(params.getParameter<std::string>("branchName")),
            doc_(params.getParameter<std::string>("docString")),
            src_(consumes<reco::CandidateView>(params.getParameter<edm::InputTag>("src"))),
            candMap_(consumes<edm::Association<reco::GenParticleCollection>>(params.getParameter<edm::InputTag>("mcMap")))
        {
            produces<FlatTable>();
            const std::string & type = params.getParameter<std::string>("objType");
            if (type == "Muon") type_ = MMuon;
            else if (type == "Electron") type_ = MElectron;
            else if (type == "Tau") type_ = MTau;
            else if (type == "Photon") type_ = MPhoton;
            else if (type == "Other") type_ = MOther;
            else throw cms::Exception("Configuration", "Unsupported objType '"+type+"'\n");
    
            switch(type_) { 
                case MMuon: flavDoc_ = "1 = prompt muon (including gamma*->mu mu), 15 = muon from prompt tau, " // continues below
                                       "5 = muon from b, 4 = muon from c, 3 = muon from light or unknown, 0 = unmatched"; break;
                case MElectron: flavDoc_ = "1 = prompt electron (including gamma*->mu mu), 15 = electron from prompt tau, 22 = prompt photon (likely conversion), " // continues below
                                           "5 = electron from b, 4 = electron from c, 3 = electron from light or unknown, 0 = unmatched"; break;
                case MPhoton: flavDoc_ = "1 = prompt photon, 13 = prompt electron, 0 = unknown or unmatched"; break;
                case MTau: flavDoc_    = "1 = prompt tau, 0 = unknown or unmatched"; break;
                case MOther: flavDoc_  = "1 = from hard scatter, 0 = unknown or unmatched"; break;
            }
        }

        virtual ~CandMCMatchTableProducer() {}

        void produce(edm::StreamID id, edm::Event& iEvent, const edm::EventSetup& iSetup) const override {

            edm::Handle<reco::CandidateView> cands;
            iEvent.getByToken(src_, cands);
            unsigned int ncand = cands->size();

            auto tab  = std::make_unique<FlatTable>(ncand, objName_, false, true);

            edm::Handle<edm::Association<reco::GenParticleCollection>> map;
            iEvent.getByToken(candMap_, map);

            std::vector<int> key(ncand, -1), flav(ncand, 0);
            for (unsigned int i = 0; i < ncand; ++i) {
                reco::GenParticleRef match = (*map)[cands->ptrAt(i)];
                if (match.isNull()) continue;
                key[i] = match.key();
                switch(type_) {
                    case MMuon:
                        if (match->isPromptFinalState()) flav[i] = 1; // prompt
                        else if (match->isDirectPromptTauDecayProductFinalState()) flav[i] = 15; // tau
                        else flav[i] = getParentHadronFlag(match); // 3 = light, 4 = charm, 5 = b
                        break;
                    case MElectron:
                        if (match->isPromptFinalState()) flav[i] = (match->pdgId() == 22 ? 22 : 1); // prompt electron or photon
                        else if (match->isDirectPromptTauDecayProductFinalState()) flav[i] = 15; // tau
                        else flav[i] = getParentHadronFlag(match); // 3 = light, 4 = charm, 5 = b
                        break;
                    case MPhoton:
                        if (match->isPromptFinalState()) flav[i] = (match->pdgId() == 22 ? 1 : 13); // prompt electron or photon
                        break;
                    case MTau:
                        if (match->isPromptDecayed()) flav[i] = 1; // FIXME: write gen decay mode? match also to light leptons?
                        break;
                    default:
                        flav[i] = match->statusFlags().fromHardProcess();
                };
            }
        
            
            tab->addColumn<int>(branchName_+"Idx",  key, "Index into genParticle list for "+doc_, FlatTable::IntColumn);
            tab->addColumn<uint8_t>(branchName_+"Flav", flav, "Flavour of genParticle for "+doc_+": "+flavDoc_, FlatTable::UInt8Column);

            iEvent.put(std::move(tab));
        }

        static int getParentHadronFlag(const reco::GenParticleRef match) {
            bool has4 = false;
            for (unsigned int im = 0, nm = match->numberOfMothers(); im < nm; ++im) {
                reco::GenParticleRef mom = match->motherRef(im);
                assert(mom.isNonnull() && mom.isAvailable()); // sanity
                if (mom.key() >= match.key()) continue; // prevent circular refs
                int id = std::abs(mom->pdgId());
                if (id / 1000 == 5 || id / 100 == 5 || id == 5) return 5;
                if (id / 1000 == 4 || id / 100 == 4 || id == 4) has4 = true;
                if (mom->status() == 2) {
                    id = getParentHadronFlag(mom);
                    if (id == 5) return 5;
                    else if (id == 4) has4 = true;
                }
            }
            return has4 ? 4 : 3;
        }

    protected:
        const std::string objName_, branchName_, doc_;
        const edm::EDGetTokenT<reco::CandidateView> src_;
        const edm::EDGetTokenT<edm::Association<reco::GenParticleCollection>> candMap_;
        enum MatchType { MMuon, MElectron, MTau, MPhoton, MOther } type_;
        std::string flavDoc_;
};

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(CandMCMatchTableProducer);

