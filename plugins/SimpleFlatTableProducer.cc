#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Common/interface/View.h"
#include "PhysicsTools/NanoAOD/interface/FlatTable.h"

#include "CommonTools/Utils/interface/StringCutObjectSelector.h"
#include "CommonTools/Utils/interface/StringObjectFunction.h"

#include <vector>

template<typename T>
class SimpleFlatTableProducer : public edm::stream::EDProducer<> {

public:

SimpleFlatTableProducer( edm::ParameterSet const & params ):
    src_(consumes<edm::View<T>>( params.getParameter<edm::InputTag>("src") )),
    cut_( params.getParameter<std::string>("cut"), true )
    {
        if (params.existsAs<edm::ParameterSet>("floats")) {
            edm::ParameterSet const & floatsPSet = params.getParameter<edm::ParameterSet>("floats");
            for (const std::string & pname : floatsPSet.getParameterNamesForType<std::string>()) {
                floats_.emplace_back(pname, StringObjectFunction<T>(floatsPSet.getParameter<std::string>(pname),true));
            }
        }
        if (params.existsAs<edm::ParameterSet>("ints")) {
            edm::ParameterSet const & intsPSet = params.getParameter<edm::ParameterSet>("ints");
            for (const std::string & pname : intsPSet.getParameterNamesForType<std::string>()) {
                ints_.emplace_back(pname, StringObjectFunction<T>(intsPSet.getParameter<std::string>(pname),true));
            }
        }
        if (params.existsAs<edm::ParameterSet>("bools")) {
            edm::ParameterSet const & boolsPSet = params.getParameter<edm::ParameterSet>("bools");
            for (const std::string & pname : boolsPSet.getParameterNamesForType<std::string>()) {
                bools_.emplace_back(pname, StringCutObjectSelector<T>(boolsPSet.getParameter<std::string>(pname),true));
            }
        }

        produces<FlatTable>();
    }
   
    virtual ~SimpleFlatTableProducer() {}
   
    virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup) override {
        
        edm::Handle<edm::View<T>> src;
        iEvent.getByToken(src_, src);

        std::vector<const T *> selobjs;
        for (const auto & obj : *src) {
            if (cut_(obj)) selobjs.push_back(&obj);
        }
        unsigned int nsel = selobjs.size();

        auto out = std::make_unique<FlatTable>(nsel);

        std::vector<float> floats(nsel);
        for (const auto & pair : floats_) {
            for (unsigned int i = 0; i < nsel; ++i) {
                floats[i] = pair.second(*selobjs[i]);
            }
            out->addColumn<float>(pair.first, floats, FlatTable::FloatColumn);
        }

        std::vector<int> ints(nsel);
        for (const auto & pair : ints_) {
            for (unsigned int i = 0; i < nsel; ++i) {
                ints[i] = pair.second(*selobjs[i]);
            }
            out->addColumn<int>(pair.first, ints, FlatTable::IntColumn);
        }

        std::vector<uint8_t> bools(nsel);
        for (const auto & pair : bools_) {
            for (unsigned int i = 0; i < nsel; ++i) {
                bools[i] = pair.second(*selobjs[i]);
            }
            out->addColumn<uint8_t>(pair.first, bools, FlatTable::UInt8Column);
        }

        iEvent.put(std::move(out));
    }


protected:
    const edm::EDGetTokenT<edm::View<T>> src_;
    const StringCutObjectSelector<T> cut_;
    
    std::vector<std::pair<std::string,StringObjectFunction<T>>> floats_; 
    std::vector<std::pair<std::string,StringObjectFunction<T>>> ints_; 
    std::vector<std::pair<std::string,StringCutObjectSelector<T>>> bools_; 
};


#include "DataFormats/Candidate/interface/Candidate.h"
typedef SimpleFlatTableProducer<reco::Candidate> SimpleCandidateFlatTableProducer;

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(SimpleCandidateFlatTableProducer);

