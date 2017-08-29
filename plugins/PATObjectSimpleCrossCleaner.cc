#include "PhysicsTools/NanoAOD/plugins/PATObjectBaseCrossCleaner.h"

class PATObjectSimpleCrossCleaner : public PATObjectBaseCrossCleaner {
public:
      PATObjectSimpleCrossCleaner(const edm::ParameterSet&p):PATObjectBaseCrossCleaner(p){}
      ~PATObjectSimpleCrossCleaner(){}

      virtual void objectSelection( const edm::View<pat::Muon>  & muons, const edm::View<pat::Jet> & jets,
                                 std::vector<uint8_t> & muonBits, std::vector<uint8_t> & jetBits ) override     {
 	    for(size_t i=0;i<jets.size();i++){
		for(const auto & m : jets[i].overlaps("muons")) {
			if(muonBits[m.key()]) jetBits[i]=0; //
		}
            }
	}
 
};
DEFINE_FWK_MODULE(PATObjectSimpleCrossCleaner);

