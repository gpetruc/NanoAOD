// -*- C++ -*-
//
// Package:     PhysicsTools/NanoAODOutput
// Class  :     NanoAODOutputModule
// 
// Implementation:
//     [Notes on implementation]
//
// Original Author:  Christopher Jones
//         Created:  Mon, 07 Aug 2017 14:21:41 GMT
//

// system include files
#include <string>
#include "TFile.h"
#include "TTree.h"

// user include files
#include "FWCore/Framework/interface/OutputModule.h"
#include "FWCore/Framework/interface/one/OutputModule.h"
#include "FWCore/Framework/interface/RunForOutput.h"
#include "FWCore/Framework/interface/LuminosityBlockForOutput.h"
#include "FWCore/Framework/interface/EventForOutput.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/JobReport.h"
#include "FWCore/Utilities/interface/GlobalIdentifier.h"
#include "FWCore/Utilities/interface/Digest.h"

#include "DataFormats/Provenance/interface/BranchDescription.h"
#include "PhysicsTools/NanoAOD/interface/FlatTable.h"
#include "PhysicsTools/NanoAOD/plugins/TableOutputBranches.h"

#include <iostream>

class NanoAODOutputModule : public edm::one::OutputModule<> {
public:
  NanoAODOutputModule(edm::ParameterSet const& pset);
  virtual ~NanoAODOutputModule();

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  virtual void write(edm::EventForOutput const& e) override;
  virtual void writeLuminosityBlock(edm::LuminosityBlockForOutput const&) override;
  virtual void writeRun(edm::RunForOutput const&) override;
  virtual bool isFileOpen() const override;
  virtual void openFile(edm::FileBlock const&) override;
  virtual void reallyCloseFile() override;

  std::string m_fileName;
  std::string m_logicalFileName;
  edm::JobReport::Token m_jrToken;
  std::unique_ptr<TFile> m_file;
  std::unique_ptr<TTree> m_tree, m_lumiTree;

  class CommonEventBranches {
     public:
         void branch(TTree &tree) {
            tree.Branch("run", & m_run, "run/i");
            tree.Branch("luminosityBlock", & m_luminosityBlock, "luminosityBlock/i");
            tree.Branch("event", & m_event, "event/l");
         }
         void fill(const edm::EventID & id) { 
            m_run = id.run(); m_luminosityBlock = id.luminosityBlock(); m_event = id.event(); 
         }
     private:
         UInt_t m_run; UInt_t m_luminosityBlock; ULong64_t m_event;
  } m_commonBranches;

  class CommonLumiBranches {
     public:
         void branch(TTree &tree) {
            tree.Branch("run", & m_run, "run/i");
            tree.Branch("luminosityBlock", & m_luminosityBlock, "luminosityBlock/i");
         }
         void fill(const edm::LuminosityBlockID & id) { 
            m_run = id.run(); 
            m_luminosityBlock = id.value(); 
         }
     private:
         UInt_t m_run; UInt_t m_luminosityBlock;
  } m_commonLumiBranches;

  std::vector<TableOutputBranches> m_tables;
};


//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
NanoAODOutputModule::NanoAODOutputModule(edm::ParameterSet const& pset):
  edm::one::OutputModuleBase::OutputModuleBase(pset),
  edm::one::OutputModule<>(pset),
  m_fileName(pset.getUntrackedParameter<std::string>("fileName")),
  m_logicalFileName(pset.getUntrackedParameter<std::string>("logicalFileName"))
{
}

NanoAODOutputModule::~NanoAODOutputModule()
{
}

void 
NanoAODOutputModule::write(edm::EventForOutput const& iEvent) {
  //Get data from 'e' and write it to the file
  edm::Service<edm::JobReport> jr;
  jr->eventWrittenToFile(m_jrToken, iEvent.id().run(), iEvent.id().event());

  m_commonBranches.fill(iEvent.id());
  for (auto & t : m_tables) t.fill(iEvent,*m_tree);
  m_tree->Fill();
}

void 
NanoAODOutputModule::writeLuminosityBlock(edm::LuminosityBlockForOutput const& iLumi) {
  edm::Service<edm::JobReport> jr;
  jr->reportLumiSection(m_jrToken, iLumi.id().run(), iLumi.id().value());

  m_commonLumiBranches.fill(iLumi.id());
  m_lumiTree->Fill();
}

void 
NanoAODOutputModule::writeRun(edm::RunForOutput const& iRun) {
  edm::Service<edm::JobReport> jr;
  jr->reportRunNumber(m_jrToken, iRun.id().run());
}

bool 
NanoAODOutputModule::isFileOpen() const {
  return nullptr != m_file.get();
}

void 
NanoAODOutputModule::openFile(edm::FileBlock const&) {
  m_file = std::make_unique<TFile>(m_fileName.c_str(),"RECREATE");
  edm::Service<edm::JobReport> jr;
  cms::Digest branchHash;
  m_jrToken = jr->outputFileOpened(m_fileName,
                                   m_logicalFileName,
                                   std::string(),
                                   "NanoAODOutputModule",
                                   description().moduleLabel(),
                                   edm::createGlobalIdentifier(),
                                   std::string(),
                                   branchHash.digest().toString(),
                                   std::vector<std::string>()
                                   );

  /* Setup file structure here */
  m_tables.clear();
  const auto & keeps = keptProducts()[0];
  m_tables.reserve(keeps.size());
  for (const auto & keep : keeps) {
      m_tables.emplace_back(keep.first, keep.second);
  }

  // create the trees
  m_tree.reset(new TTree("Events","Events"));
  m_tree->SetAutoSave(std::numeric_limits<Long64_t>::max());
  m_commonBranches.branch(*m_tree);

  m_lumiTree.reset(new TTree("LuminosityBlocks","LuminosityBlocks"));
  m_lumiTree->SetAutoSave(std::numeric_limits<Long64_t>::max());
  m_commonLumiBranches.branch(*m_lumiTree);
}
void 
NanoAODOutputModule::reallyCloseFile() {
  m_file->Write();
  m_file->Close();
  m_file.reset();
  m_tree.release();     // apparently root has ownership
  m_lumiTree.release(); // 
  edm::Service<edm::JobReport> jr;
  jr->outputFileClosed(m_jrToken);
}

void 
NanoAODOutputModule::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;

  desc.addUntracked<std::string>("fileName");
  desc.addUntracked<std::string>("logicalFileName","");

  //replace with whatever you want to get from the EDM by default
  const std::vector<std::string> keep = {"drop *", "keep FlatTable_*_*_*"};
  edm::OutputModule::fillDescription(desc, keep);
  
  //Used by Workflow management for their own meta data
  edm::ParameterSetDescription dataSet;
  dataSet.setAllowAnything();
  desc.addUntracked<edm::ParameterSetDescription>("dataset", dataSet)
    ->setComment("PSet is only used by Data Operations and not by this module.");
  
  edm::ParameterSetDescription branchSet;
  branchSet.setAllowAnything();
  desc.add<edm::ParameterSetDescription>("branches", branchSet);

  descriptions.addDefault(desc);

}

DEFINE_FWK_MODULE(NanoAODOutputModule);
