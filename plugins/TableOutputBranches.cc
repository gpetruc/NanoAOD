#include "PhysicsTools/NanoAOD/plugins/TableOutputBranches.h"

#include <iostream>

void 
TableOutputBranches::defineBranchesFromFirstEvent(const FlatTable & tab) 
{
    m_baseName=tab.name();
    for(size_t i=0;i<tab.nColumns();i++){
        const std::string & var=tab.columnName(i);
        switch(tab.columnType(i)){
            case (FlatTable::FloatColumn):
                m_floatBranches.emplace_back(var, tab.columnDoc(i));
                break;
            case (FlatTable::IntColumn):
                m_intBranches.emplace_back(var, tab.columnDoc(i));
                break;
            case (FlatTable::UInt8Column):
                m_uint8Branches.emplace_back(var, tab.columnDoc(i));
                break;
        }
    }
}

void 
TableOutputBranches::branch(TTree &tree) 
{
    if (!m_singleton)  {
        if(tree.FindBranch(("n"+m_baseName).c_str())!=nullptr)
        {
            //FIXME
            std::cout << "Multiple tables providing " << m_baseName << ": need to implement a safety check on the sizes" << std::endl;
        } else {
            tree.Branch(("n"+m_baseName).c_str(), & m_counter, ("n"+m_baseName + "/i").c_str());
        }
    }
    std::string varsize = m_singleton ? "" : "[n" + m_baseName + "]";
    for (auto & pair : m_floatBranches) {
        std::string branchName = (!m_baseName.empty()) ? m_baseName + "_" + pair.name : pair.name;
        pair.branch = tree.Branch(branchName.c_str(), (void*)nullptr, (branchName + varsize + "/F").c_str());
        pair.branch->SetTitle(pair.title.c_str());
    }
    for (auto & pair : m_intBranches) {
        std::string branchName = (!m_baseName.empty()) ? m_baseName + "_" + pair.name : pair.name;
        pair.branch = tree.Branch(branchName.c_str(), (void*)nullptr, (branchName + varsize + "/I").c_str());
        pair.branch->SetTitle(pair.title.c_str());
    }
    for (auto & pair : m_uint8Branches) {
        std::string branchName = (!m_baseName.empty()) ? m_baseName + "_" + pair.name : pair.name;
        pair.branch = tree.Branch(branchName.c_str(), (void*)nullptr, (branchName + varsize + "/b").c_str());
        pair.branch->SetTitle(pair.title.c_str());
    }
}

void TableOutputBranches::fill(const edm::EventForOutput &iEvent,TTree & tree) 
{
    edm::Handle<FlatTable> handle;
    iEvent.getByToken(m_token, handle);
    const FlatTable & tab = *handle;
    m_counter = tab.size();
    m_singleton = tab.singleton();
    if(!m_branchesBooked) {
        defineBranchesFromFirstEvent(tab);	
        m_branchesBooked=true;
        branch(tree); 
    }
    for (auto & pair : m_floatBranches) fillColumn<float>(pair, tab);
    for (auto & pair : m_intBranches) fillColumn<int>(pair, tab);
    for (auto & pair : m_uint8Branches) fillColumn<uint8_t>(pair, tab);
}


