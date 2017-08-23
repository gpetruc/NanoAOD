#include "Rtypes.h" 

#include <PhysicsTools/NanoAOD/interface/FlatTable.h>
#include "DataFormats/Common/interface/Wrapper.h"

namespace PhysicsTools_NanoAOD {
    struct dictionary {
        edm::Wrapper<FlatTable> w_table;
    };
}
