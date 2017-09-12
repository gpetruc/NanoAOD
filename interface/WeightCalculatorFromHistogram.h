#ifndef PhysicsTools_NanoAOD_WeightCalculatorFromHistogram_h
#define PhysicsTools_NanoAOD_WeightCalculatorFromHistogram_h

#include <TH1.h>

class WeightCalculatorFromHistogram {
 public:
  WeightCalculatorFromHistogram() {}
 WeightCalculatorFromHistogram(TH1 *histogram, bool verbose=false) : histogram_(histogram), verbose_(verbose) {}
  ~WeightCalculatorFromHistogram() {}
  
  float getWeight(float x, float y=0) const;
  float getWeightErr(float x, float y=0) const;
  
 private:
  TH1* histogram_;
  bool verbose_;
};

#endif

