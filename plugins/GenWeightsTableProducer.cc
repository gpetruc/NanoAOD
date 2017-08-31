#include "FWCore/Framework/interface/global/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "PhysicsTools/NanoAOD/interface/FlatTable.h"
#include "PhysicsTools/NanoAOD/interface/MergableCounterTable.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"

#include <vector>

namespace {
    struct Counter {
        Counter() : num(0), sumw(0), sumw2(0) {}
        long long num;
        long double sumw;
        long double sumw2;
        void clear() { num = 0; sumw = 0; sumw2 = 0; }
        void inc(double w) { num++; sumw += w; sumw2 += (w*w); }
        void inc(const Counter & other) { num += other.num; sumw += other.sumw; sumw2 += other.sumw2; } 
    };
}
class GenWeightsTableProducer : public edm::global::EDProducer<edm::StreamCache<Counter>, edm::RunSummaryCache<Counter>, edm::EndRunProducer> {
    public:

        GenWeightsTableProducer( edm::ParameterSet const & params ) :
            name_(params.getParameter<std::string>("name")),
            doc_(params.getParameter<std::string>("doc")),
            genTag_(consumes<GenEventInfoProduct>(params.getParameter<edm::InputTag>("genEvent")))
        {
            produces<FlatTable>();
            produces<MergableCounterTable,edm::InRun>();
        }

        virtual ~GenWeightsTableProducer() {}

        void produce(edm::StreamID id, edm::Event& iEvent, const edm::EventSetup& iSetup) const override {
            auto out = std::make_unique<FlatTable>(1, name_, true);
            out->setDoc(doc_);

            edm::Handle<GenEventInfoProduct> genInfo;
            iEvent.getByToken(genTag_, genInfo);
            
            double weight = genInfo->weight();
            streamCache(id)->inc(weight);
 
            out->addColumnValue<float>("", weight, doc_, FlatTable::FloatColumn);

            iEvent.put(std::move(out));
        }

        // create an empty counter
        std::unique_ptr<Counter> beginStream(edm::StreamID) const override { 
            return std::make_unique<Counter>(); 
        }
        // inizialize to zero at begin run
        void streamBeginRun(edm::StreamID id, edm::Run const&, edm::EventSetup const&) const override { 
            streamCache(id)->clear(); 
        }
        // create an empty counter
        std::shared_ptr<Counter> globalBeginRunSummary(edm::Run const&, edm::EventSetup const&) const override { 
            return std::make_shared<Counter>(); 
        }
        // add this stream to the summary
        void streamEndRunSummary(edm::StreamID id, edm::Run const&, edm::EventSetup const&, Counter* runCounter) const override { 
            runCounter->inc(*streamCache(id)); 
        }
        // nothing to do per se
        void globalEndRunSummary(edm::Run const&, edm::EventSetup const&, Counter* runCounter) const override { 
        }
        // write the total to the run 
        void globalEndRunProduce(edm::Run& iRun, edm::EventSetup const&, Counter const* runCounter) const override {
            auto out = std::make_unique<MergableCounterTable>();
            out->addInt(name_+"Count", "event count for "+doc_, runCounter->num);
            out->addFloat(name_+"Sumw", "sum of weights for "+doc_, runCounter->sumw);
            out->addFloat(name_+"Sumw2", "sum of (weight^2) for "+doc_, runCounter->sumw2);
            iRun.put(std::move(out));
        }
    protected:
        const std::string name_, doc_;
        const edm::EDGetTokenT<GenEventInfoProduct> genTag_;
};

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(GenWeightsTableProducer);

