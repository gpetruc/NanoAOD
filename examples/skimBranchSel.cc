#include <iostream>
#include "TTree.h"
#include "TFile.h"
#include "TH1F.h"

void skimBranchSel(std::string infile,std::string cut="Jet_pt>200",std::string branchtoselect="*",bool selectedAreKept=true)
{
  TFile *file=new TFile((infile+".root").c_str());
  TTree *events=(TTree*)file->Get("Events");
  TTree *lumi=(TTree*)file->Get("LuminosityBlocks");
  TTree *runs=(TTree*)file->Get("Runs");
  gBenchmark->Start("skim");
  events->SetBranchStatus("*",!selectedAreKept); 
  events->SetBranchStatus(branchtoselect.c_str(),selectedAreKept);
 
  std::cout<<"Copying new tree with "<<cut<<"... ";
  TFile *outfile=new TFile((infile+"_Skim.root").c_str(), "recreate","",9);
  outfile->cd();
  TTree *events_new=(TTree*)events->CopyTree(cut.c_str());
  TTree *lumi_new=(TTree*)lumi->Clone();
  TTree *runs_new=(TTree*)runs->Clone();
  std::cout<<"done."<<std::endl;


  events_new->Write();
  lumi_new->Write();
  runs_new->Write();
  std::cout << "Input events: " << events->GetEntries() << " output Events: " << events_new->GetEntries() << "  skim efficiency " << 1.*events_new->GetEntries()/events->GetEntries() << std::endl;
  outfile->Close();
  gBenchmark->Print("skim");
  
  return;
} 
