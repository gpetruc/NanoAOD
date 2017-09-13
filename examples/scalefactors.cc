#include <vector>
#include <string>
#include <iostream>

#include <TSystem.h>
#include <TFile.h>
#include <TH2.h>
#include <TFile.h>
#include <TTree.h>

#include "helpers/LeptonEfficiencyCorrector.cc"

std::vector<std::string> electron_eff_files = 
  {"../data/leptonSF/EGM2D_eleGSF.root",
   "../data/leptonSF/EGM2D_eleMVA90.root",
  };
std::vector<std::string> electron_eff_histos = {"EGamma_SF2D", "EGamma_SF2D"};

std::vector<std::string> muon_eff_files = 
  {"../data/leptonSF/Mu_Trg.root",
   "../data/leptonSF/Mu_ID.root",
   "../data/leptonSF/Mu_Iso.root"
  };
std::vector<std::string> muon_eff_histos = 
  {"IsoMu24_OR_IsoTkMu24_PtEtaBins/pt_abseta_ratio",
   "MC_NUM_LooseID_DEN_genTracks_PAR_pt_eta/pt_abseta_ratio",
   "LooseISO_LooseID_pt_eta/pt_abseta_ratio"};

LeptonEfficiencyCorrector el_sf(electron_eff_files,electron_eff_histos);
LeptonEfficiencyCorrector mu_sf(muon_eff_files,muon_eff_histos);

void scalefactors(std::string infile) {

  TFile *file=new TFile((infile+".root").c_str());
  TTree *events=(TTree*)file->Get("Events");
  TFile *ff = new TFile((infile+"_friend.root").c_str(),"recreate");
  TTree *TF = new TTree("TF","scale factors friend tree");

  const UInt_t maxLepSize=10;
  UInt_t          nElectron;
  Float_t         Electron_pt[maxLepSize];
  Float_t         Electron_eta[maxLepSize];
  UInt_t          nMuon;
  Float_t         Muon_pt[maxLepSize];
  Float_t         Muon_eta[maxLepSize];

  events->SetBranchAddress("nElectron", &nElectron);
  events->SetBranchAddress("Electron_pt", Electron_pt);
  events->SetBranchAddress("Electron_eta", Electron_eta);
  events->SetBranchAddress("nMuon", &nMuon);
  events->SetBranchAddress("Muon_pt", Muon_pt);
  events->SetBranchAddress("Muon_eta", Muon_eta);

  float Electron_sf[maxLepSize],Muon_sf[maxLepSize];
  TF->Branch("nElectron",&nElectron,"nElectron/i");
  TF->Branch("nMuon",&nMuon,"nMuon/i");
  TF->Branch("Electron_sf",Electron_sf,"Electron_sf[nElectron]/F");
  TF->Branch("Muon_sf",Muon_sf,"Muon_sf[nMuon]/F");

  for (Int_t i=0;i<events->GetEntries();i++) {
    if(i%100==0) std::cout << "Processed " << i << " entries." << std::endl;
    events->GetEntry(i);
    for(unsigned int el=0; el<std::min(maxLepSize,nElectron); ++el) {
      Electron_sf[el] = el_sf.getSF(11,Electron_pt[el],Electron_eta[el]);
    }
    for(unsigned int mu=0; mu<std::min(maxLepSize,nMuon); ++mu) {
      Muon_sf[mu] = mu_sf.getSF(13,Muon_pt[mu],Muon_eta[mu]);
    }
    TF->Fill();
  }
  ff->cd();
  TF->Write();
  ff->Close();
}


float leptonSF(int pdgid, float pt, float eta) {
  if(abs(pdgid)==11) return el_sf.getSF(pdgid,pt,eta);
  else if(abs(pdgid)==13) return mu_sf.getSF(pdgid,pt,eta);
  else return 0;
}
