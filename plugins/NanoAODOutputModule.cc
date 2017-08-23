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
  edm::ParameterSet branchPSet;

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

  class TableOutputBranches {
     public:
        TableOutputBranches(const edm::BranchDescription *desc, const edm::EDGetToken & token, const edm::ParameterSet &mainPSet) :
            m_token(token)
        {
            if (desc->className() != "FlatTable") throw cms::Exception("Configuration", "NanoAODOutputModule can only write out FlatTable objects");
            std::string pname = desc->moduleLabel();
            if (!desc->productInstanceName().empty()) pname += "_"+desc->productInstanceName();
            const edm::ParameterSet & pset = mainPSet.getParameter<edm::ParameterSet>(pname);
            m_baseName = pset.getParameter<std::string>("baseName");
            m_buffer = FlatTable(pset.getParameter<uint32_t>("maxEntries"));
            typedef std::vector<std::string> vstring;
            if (pset.existsAs<vstring>("floats")) {
                std::vector<float> zeros(m_buffer.size());
                for (const std::string & fvar : pset.getParameter<vstring>("floats")) {
                    m_buffer.addColumn<float>(fvar, zeros, FlatTable::FloatColumn);
                }
            }
            if (pset.existsAs<vstring>("ints")) {
                std::vector<int> zeros(m_buffer.size());
                for (const std::string & fvar : pset.getParameter<vstring>("ints")) {
                    m_buffer.addColumn<int>(fvar, zeros, FlatTable::IntColumn);
                }
            }
            if (pset.existsAs<vstring>("uint8s")) {
                std::vector<uint8_t> zeros(m_buffer.size());
                for (const std::string & fvar : pset.getParameter<vstring>("uint8s")) {
                    m_buffer.addColumn<uint8_t>(fvar, zeros, FlatTable::UInt8Column);
                }
            }
        }
        void branch(TTree &tree) {
            tree.Branch(("n"+m_baseName).c_str(), & m_counter, ("n"+m_baseName + "/i").c_str());
            std::string varsize = "[n" + m_baseName + "]";
            for (unsigned int i = 0, n = m_buffer.nColumns(); i < n; ++i) {
                std::string branchName = m_baseName + "_" + m_buffer.columnName(i);
                switch(m_buffer.columnType(i)) {
                    case FlatTable::FloatColumn: tree.Branch(branchName.c_str(), & m_buffer.columnData<float>(i).front(),   (branchName + varsize + "/F").c_str()); break;
                    case FlatTable::IntColumn:   tree.Branch(branchName.c_str(), & m_buffer.columnData<int  >(i).front(),   (branchName + varsize + "/I").c_str()); break;
                    case FlatTable::UInt8Column: tree.Branch(branchName.c_str(), & m_buffer.columnData<uint8_t>(i).front(), (branchName + varsize + "/b").c_str()); break;
                }
            }
        }
        void fill(const edm::EventForOutput &iEvent) {
            edm::Handle<FlatTable> handle;
            iEvent.getByToken(m_token, handle);
            const FlatTable & tab = *handle;
            m_counter = std::min(tab.size(), m_buffer.size());
            for (unsigned int i = 0, n = m_buffer.nColumns(); i < n; ++i) {
                switch(m_buffer.columnType(i)) {
                    case FlatTable::FloatColumn: fillColumn<float  >(i, tab); break;
                    case FlatTable::IntColumn:   fillColumn<int    >(i, tab); break;
                    case FlatTable::UInt8Column: fillColumn<uint8_t>(i, tab, 0); break;
                }
            }
        }
        template<typename T>
        void fillColumn(int col, const FlatTable & tab, T defval=-99) {
            int idx = tab.columnIndex(m_buffer.columnName(col));
            if (idx == -1) throw cms::Exception("LogicError", "Missing column in input for "+m_baseName+"_"+m_buffer.columnName(col));
            if (tab.columnType(idx) != m_buffer.columnType(col)) throw cms::Exception("LogicError", "Column type mismatch for "+m_baseName+"_"+m_buffer.columnName(col));
            auto out = m_buffer.columnData<T>(col);
            auto  in = tab.columnData<T>(idx);
            std::copy_n(in.begin(), m_counter, out.begin());
            std::fill(out.begin()+m_counter, out.end(), defval);
        }
     private:
        edm::EDGetToken m_token;
        std::string  m_baseName;
        unsigned int m_maxEntries;
        FlatTable    m_buffer;
        UInt_t       m_counter;
  };
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
  m_logicalFileName(pset.getUntrackedParameter<std::string>("logicalFileName")),
  branchPSet(pset.getParameter<edm::ParameterSet>("branches"))
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
  for (auto & t : m_tables) t.fill(iEvent);
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
      m_tables.emplace_back(keep.first, keep.second, branchPSet);
  }

  // create the trees
  m_tree.reset(new TTree("Events","Events"));
  m_tree->SetAutoSave(std::numeric_limits<Long64_t>::max());
  m_commonBranches.branch(*m_tree);
  for (auto & t : m_tables) t.branch(*m_tree);

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
