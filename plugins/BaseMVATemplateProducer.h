#ifndef PhysicsTools_NanoAOD_BaseMVATemplateProducer
#define PhysicsTools_NanoAOD_BaseMVATemplateProducer

// -*- C++ -*-
//
// Package:    PhysicsTools/NanoAOD
// Class:      BaseMVATemplateProducer
// 
/**\class BaseMVATemplateProducer BaseMVATemplateProducer.cc PhysicsTools/NanoAOD/plugins/BaseMVATemplateProducer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Andre Rizzi
//         Created:  Mon, 07 Sep 2017 09:18:03 GMT
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


#include "TMVA/Factory.h"
#include "TMVA/Reader.h"

#include "CommonTools/Utils/interface/StringObjectFunction.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "PhysicsTools/NanoAOD/interface/FlatTable.h"
#include <string>
//
// class declaration
//

template <typename T,typename O>
class BaseMVATemplateProducer : public edm::stream::EDProducer<> {
   public:
  explicit BaseMVATemplateProducer(const edm::ParameterSet &iConfig):
    src_(consumes<edm::View<T>>(iConfig.getParameter<edm::InputTag>("src"))),
    variablesOrder_(iConfig.getParameter<std::vector<std::string>>("variablesOrder")),
    attName_(iConfig.getParameter<std::string>("attName")),
    collName_(iConfig.getParameter<std::string>("collName"))
  {
      std::cout << "hello" << std::endl;
      edm::ParameterSet const & varsPSet = iConfig.getParameter<edm::ParameterSet>("variables");
      for (const std::string & vname : varsPSet.getParameterNamesForType<std::string>()) {
      	  std::cout << vname << std::endl;
	  funcs_.emplace_back(std::pair<std::string,StringObjectFunction<T>>(vname,varsPSet.getParameter<std::string>(vname)));
      }

      values_.resize(variablesOrder_.size());
      size_t i=0;
      for(const auto & v : variablesOrder_){
	positions_[v]=i;
	reader_.AddVariable(v,(&values_.front())+i);
	i++;
      }
      reader_.BookMVA(collName_+"_"+attName_,iConfig.getParameter<edm::FileInPath>("weightFile").fullPath() );
      produces<O>();

  }
  ~BaseMVATemplateProducer() {}

  void setValue(const std::string var,float val) {
	values_[positions_[var]]=val;
  }
  
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

   private:
  virtual void beginStream(edm::StreamID) override {};
  virtual void produce(edm::Event&, const edm::EventSetup&) override;
  virtual void endStream() override {};

  ///to be implemented in derived classes, filling values for additional variables
  virtual void readAdditionalCollections(edm::Event&, const edm::EventSetup&)  {}
  virtual void fillAdditionalVariables(const T&)  {}


  edm::ValueMap<float> * prepareOutput(edm::Handle<edm::View<T>> &src,edm::ValueMap<float>&, const std::vector<float> & vals); 
  FlatTable * prepareOutput(edm::Handle<edm::View<T>> &src,FlatTable&, const std::vector<float> & vals) ;

  edm::EDGetTokenT<edm::View<T>> src_;
  std::map<std::string,size_t> positions_;
  std::vector<std::pair<std::string,StringObjectFunction<T>>> funcs_;
  std::vector<std::string> variablesOrder_;
  std::vector<float> values_;
  TMVA::Reader reader_;
  std::string attName_;
  std::string collName_;

};

template <typename T,typename O>
void
BaseMVATemplateProducer<T,O>::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  edm::Handle<edm::View<T>> src;
  iEvent.getByToken(src_, src);
  readAdditionalCollections(iEvent,iSetup);
  
  std::vector<float> mvaOut;
  mvaOut.reserve(src->size());
  for(auto const & o: *src) {
	for(auto const & p : funcs_ ){
		values_[positions_[p.first]]=p.second(o);
	}
        fillAdditionalVariables(o);
	mvaOut.push_back(reader_.EvaluateRegression(collName_+"_"+attName_)[0]);
  }
  O dumb;
  std::unique_ptr<O> mvaV(prepareOutput(src,dumb,mvaOut));

  iEvent.put(std::move(mvaV));

}

template <typename T,typename O>
void
BaseMVATemplateProducer<T,O>::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

template <typename T,typename O> 
 edm::ValueMap<float> *
BaseMVATemplateProducer<T,O>::prepareOutput(edm::Handle<edm::View<T>> &src, edm::ValueMap<float> &, const std::vector<float> & vals)
{
  edm::ValueMap<float> * mvaV= new edm::ValueMap<float>();
  edm::ValueMap<float>::Filler filler(*mvaV);
  filler.insert(src,vals.begin(),vals.end());
  filler.fill();
  return mvaV;
}
template <typename T,typename O> 
FlatTable *
BaseMVATemplateProducer<T,O>::prepareOutput(edm::Handle<edm::View<T>> &src, FlatTable &, const std::vector<float> & vals)
{
  FlatTable * mvaV= new FlatTable(src->size(),collName_,false,true);
  mvaV->addColumn<float>(attName_,vals,attName_,FlatTable::FloatColumn,10);

  return mvaV;
}



#endif
