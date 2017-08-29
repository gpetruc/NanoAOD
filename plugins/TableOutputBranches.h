#ifndef PhysicsTools_NanoAOD_TableOutputBranches_h
#define PhysicsTools_NanoAOD_TableOutputBranches_h

#include <string>
#include <vector>
#include <TTree.h>
#include "FWCore/Framework/interface/EventForOutput.h"
#include "PhysicsTools/NanoAOD/interface/FlatTable.h"
#include "DataFormats/Provenance/interface/BranchDescription.h"
#include "FWCore/Utilities/interface/EDGetToken.h"

class TableOutputBranches {
 public:
    TableOutputBranches(const edm::BranchDescription *desc, const edm::EDGetToken & token ) :
        m_token(token), m_branchesBooked(false)
    {
        if (desc->className() != "FlatTable") throw cms::Exception("Configuration", "NanoAODOutputModule can only write out FlatTable objects");
    }

    void defineBranchesFromFirstEvent(const FlatTable & tab) ;
    void branch(TTree &tree) ;
    void fill(const edm::EventForOutput &iEvent,TTree & tree) ;

 private:
    edm::EDGetToken m_token;
    std::string  m_baseName;
    bool         m_singleton;
    UInt_t       m_counter;
    struct NamedBranchPtr {
        std::string name, title;
        TBranch * branch;
        NamedBranchPtr(const std::string & aname, const std::string & atitle, TBranch *branchptr = nullptr) : 
            name(aname), title(atitle), branch(branchptr) {}
    };
    std::vector<NamedBranchPtr> m_floatBranches;
    std::vector<NamedBranchPtr>   m_intBranches;
    std::vector<NamedBranchPtr> m_uint8Branches;
    bool m_branchesBooked;

    template<typename T>
    void fillColumn(NamedBranchPtr & pair, const FlatTable & tab) {
        int idx = tab.columnIndex(pair.name);
        if (idx == -1) throw cms::Exception("LogicError", "Missing column in input for "+m_baseName+"_"+pair.name);
        pair.branch->SetAddress( const_cast<T *>(& tab.columnData<T>(idx).front() ) ); // SetAddress should take a const * !
    }

};

#endif

