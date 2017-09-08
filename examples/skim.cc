#include <iostream>
#include "TTree.h"
#include "TFile.h"
#include "TH1F.h"

void skim(std::string infile,std::string cut="Jet_pt>200")
{
  TFile *file=new TFile((infile+".root").c_str());
  TTree *events=(TTree*)file->Get("Events");
  TTree *lumi=(TTree*)file->Get("LuminosityBlocks");
  TTree *runs=(TTree*)file->Get("Runs");
  gBenchmark->Start("skim");
  
  std::cout<<"Copying new tree with "<<cut<<"... ";
  TFile *outfile=new TFile((infile+"_Skim.root").c_str(), "recreate","",9);
  outfile->cd();
  TTree *events_new=(TTree*)events->CopyTree(cut.c_str());
  TTree *lumi_new=(TTree*)lumi->Clone();
  TTree *runs_new=(TTree*)runs->Clone();
  std::cout<<"done cloning"<<std::endl;
  events_new->Write();
  lumi_new->Write();
  runs_new->Write();
  gBenchmark->Show("skim");
  std::cout << "Input events: " << events->GetEntries() << " output Events: " << events_new->GetEntries() << "  skim efficiency " << 1.*events_new->GetEntries()/events->GetEntries() << std::endl;
  std::cout << "Input rate: " << 0.001*events->GetEntries()/gBenchmark->GetCpuTime("skim") << " KHz,  output Events: " << 1.*events_new->GetEntries()/gBenchmark->GetCpuTime("skim") << " Hz " << std::endl;
  outfile->Close();
  
  return;
} 
