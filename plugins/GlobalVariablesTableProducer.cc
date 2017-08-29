#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "PhysicsTools/NanoAOD/interface/FlatTable.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"

#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>

class GlobalVariablesTableProducer : public edm::stream::EDProducer<> {
    public:

        GlobalVariablesTableProducer( edm::ParameterSet const & params )
        {
            edm::ParameterSet const & varsPSet = params.getParameter<edm::ParameterSet>("variables");
            for (const std::string & vname : varsPSet.getParameterNamesForType<edm::ParameterSet>()) {
                const auto & varPSet = varsPSet.getParameter<edm::ParameterSet>(vname);
                const std::string & type = varPSet.getParameter<std::string>("type");
                if (type == "int") vars_.push_back(new IntVar(vname, FlatTable::IntColumn, varPSet, consumesCollector()));
                else if (type == "float") vars_.push_back(new FloatVar(vname, FlatTable::FloatColumn, varPSet, consumesCollector()));
                else if (type == "double") vars_.push_back(new DoubleVar(vname, FlatTable::FloatColumn, varPSet, consumesCollector()));
                else if (type == "bool") vars_.push_back(new BoolVar(vname, FlatTable::UInt8Column, varPSet, consumesCollector()));
                else throw cms::Exception("Configuration", "unsupported type "+type+" for variable "+vname);
            }

            produces<FlatTable>();
        }

        virtual ~GlobalVariablesTableProducer() {}

        void produce(edm::Event& iEvent, const edm::EventSetup& iSetup) override {
            auto out = std::make_unique<FlatTable>(1, "", true);

            for (const auto & var : vars_) var.fill(iEvent, *out);

            iEvent.put(std::move(out));
        }

    protected:
        class Variable {
            public:
                Variable(const std::string & aname, FlatTable::ColumnType atype, const edm::ParameterSet & cfg) : 
                    name_(aname), doc_(cfg.getParameter<std::string>("doc")), type_(atype) {}
                virtual void fill(const edm::Event &iEvent, FlatTable & out) const = 0;
                virtual ~Variable() {}
                const std::string & name() const { return name_; }
                const FlatTable::ColumnType & type() const { return type_; }
            protected:
                std::string name_, doc_;
                FlatTable::ColumnType type_;
        };
        template<typename ValType, typename ColType=ValType>
            class VariableT : public Variable {
                public:
                    VariableT(const std::string & aname, FlatTable::ColumnType atype, const edm::ParameterSet & cfg, edm::ConsumesCollector && cc) :
                        Variable(aname, atype, cfg), src_(cc.consumes<ValType>(cfg.getParameter<edm::InputTag>("src"))) {}
                    ~VariableT() {}
                    void fill(const edm::Event &iEvent, FlatTable & out) const override {
                        edm::Handle<ValType> handle;
                        iEvent.getByToken(src_, handle);
                        out.template addColumnValue<ColType>(this->name_, *handle, this->doc_, this->type_);
                    }
                protected:
                    edm::EDGetTokenT<ValType> src_;
            };
        typedef VariableT<int> IntVar;
        typedef VariableT<float> FloatVar;
        typedef VariableT<double,float> DoubleVar;
        typedef VariableT<bool,uint8_t> BoolVar;
        boost::ptr_vector<Variable> vars_;
};

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(GlobalVariablesTableProducer);

