#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Common/interface/View.h"
#include "PhysicsTools/NanoAOD/interface/FlatTable.h"

#include "CommonTools/Utils/interface/StringCutObjectSelector.h"
#include "CommonTools/Utils/interface/StringObjectFunction.h"

#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>

template<typename T, typename TProd>
class SimpleFlatTableProducerBase : public edm::stream::EDProducer<> {
    public:

        SimpleFlatTableProducerBase( edm::ParameterSet const & params ):
            name_( params.getParameter<std::string>("name") ),
            doc_(params.existsAs<std::string>("doc") ? params.getParameter<std::string>("doc") : ""),
            extension_(params.existsAs<bool>("extension") ? params.getParameter<bool>("extension") : false),
            src_(consumes<TProd>( params.getParameter<edm::InputTag>("src") )) 
        {
            edm::ParameterSet const & varsPSet = params.getParameter<edm::ParameterSet>("variables");
            for (const std::string & vname : varsPSet.getParameterNamesForType<edm::ParameterSet>()) {
                const auto & varPSet = varsPSet.getParameter<edm::ParameterSet>(vname);
                const std::string & type = varPSet.getParameter<std::string>("type");
                if (type == "int") vars_.push_back(new IntVar(vname, FlatTable::IntColumn, varPSet));
                else if (type == "float") vars_.push_back(new FloatVar(vname, FlatTable::FloatColumn, varPSet));
                else if (type == "bool") vars_.push_back(new BoolVar(vname, FlatTable::UInt8Column, varPSet));
                else throw cms::Exception("Configuration", "unsupported type "+type+" for variable "+vname);
            }

            produces<FlatTable>();
        }

        virtual ~SimpleFlatTableProducerBase() {}

        // these two are to be overriden by the child class
        virtual bool singleton() const = 0; 
        virtual void getObjects(const TProd & prod, std::vector<const T *> & selobjs) const = 0;

        void produce(edm::Event& iEvent, const edm::EventSetup& iSetup) override {
            edm::Handle<TProd> src;
            iEvent.getByToken(src_, src);

            std::vector<const T *> selobjs;
            getObjects(*src, selobjs);

            auto out = std::make_unique<FlatTable>(selobjs.size(), name_, singleton(), extension_);
            out->setDoc(doc_);

            for (const auto & var : vars_) var.fill(selobjs, *out);

            iEvent.put(std::move(out));
        }

    protected:
        const std::string name_; 
        const std::string doc_;
        const bool extension_;
        const edm::EDGetTokenT<TProd> src_;

        class Variable {
            public:
                Variable(const std::string & aname, FlatTable::ColumnType atype, const edm::ParameterSet & cfg) : 
                    name_(aname), doc_(cfg.getParameter<std::string>("doc")), type_(atype),
		    precision_(cfg.existsAs<int>("precision") ? cfg.getParameter<int>("precision") : -1)
            {
            }
                virtual void fill(std::vector<const T *> selobjs, FlatTable & out) const = 0;
                virtual ~Variable() {}
                const std::string & name() const { return name_; }
                const FlatTable::ColumnType & type() const { return type_; }
            protected:
                std::string name_, doc_;
                FlatTable::ColumnType type_;
		int precision_;
        };
        template<typename StringFunctor, typename ValType>
            class FuncVariable : public Variable {
                public:
                    FuncVariable(const std::string & aname, FlatTable::ColumnType atype, const edm::ParameterSet & cfg) :
                        Variable(aname, atype, cfg), func_(cfg.getParameter<std::string>("expr"), true) {}
                    ~FuncVariable() {}
                    void fill(std::vector<const T *> selobjs, FlatTable & out) const override {
                        std::vector<ValType> vals(selobjs.size());
                        for (unsigned int i = 0, n = vals.size(); i < n; ++i) {
                            vals[i] = func_(*selobjs[i]);
                        }
                        out.template addColumn<ValType>(this->name_, vals, this->doc_, this->type_,this->precision_);
                    }
                protected:
                    StringFunctor func_;

            };
        typedef FuncVariable<StringObjectFunction<T>,int> IntVar;
        typedef FuncVariable<StringObjectFunction<T>,float> FloatVar;
        typedef FuncVariable<StringCutObjectSelector<T>,uint8_t> BoolVar;
        boost::ptr_vector<Variable> vars_;
};

template<typename T>
class SimpleFlatTableProducer : public SimpleFlatTableProducerBase<T, edm::View<T>> {
    public:
        SimpleFlatTableProducer( edm::ParameterSet const & params ) :
            SimpleFlatTableProducerBase<T, edm::View<T>>(params),
            singleton_(params.getParameter<bool>("singleton")),
            cut_(!singleton_ ? params.getParameter<std::string>("cut") : "", true) {}

        virtual ~SimpleFlatTableProducer() {}

        bool singleton() const override { return singleton_; } 

        void getObjects(const edm::View<T> &src, std::vector<const T *> & selobjs) const override {
            if (singleton_) { 
                assert(src.size() == 1);
                selobjs.push_back(& src[0] );
            } else {
                for (const auto & obj : src) {
                    if (cut_(obj)) selobjs.push_back(&obj);
                }
            }
        } 

    protected:
        bool  singleton_;
        const StringCutObjectSelector<T> cut_;
};

template<typename T>
class EventSingletonSimpleFlatTableProducer : public SimpleFlatTableProducerBase<T,T> {
    public:
        EventSingletonSimpleFlatTableProducer( edm::ParameterSet const & params ):
            SimpleFlatTableProducerBase<T,T>(params) {}

        virtual ~EventSingletonSimpleFlatTableProducer() {}

        bool singleton() const override { return true; }

        void getObjects(const T & obj, std::vector<const T *> & selobjs) const override {
            selobjs.push_back(&obj);
        }
};

#include "DataFormats/Candidate/interface/Candidate.h"
typedef SimpleFlatTableProducer<reco::Candidate> SimpleCandidateFlatTableProducer;

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(SimpleCandidateFlatTableProducer);

