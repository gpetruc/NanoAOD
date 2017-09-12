#ifndef PhysicsTools_NanoAOD_LeptonEfficiencyCorrector_h
#define PhysicsTools_NanoAOD_LeptonEfficiencyCorrector_h

#include <string>
#include <vector>
#include <TH2.h>
#include <TFile.h>

class LeptonEfficiencyCorrector {
 public:
  LeptonEfficiencyCorrector() {effmaps_.clear();}
  LeptonEfficiencyCorrector(std::vector<std::string> files, std::vector<std::string> histos);
  ~LeptonEfficiencyCorrector() {}

  float getSF(int pdgid, float pt, float eta);
  float getSFErr(int pdgid, float pt, float eta);

 private:
  std::vector<TH2F*> effmaps_;
};

#endif

