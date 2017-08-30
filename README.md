# NanoAOD
Development repository for a new flat cms dataformat (will be merged in CMSSW once the prototyping is over)

## Current recipe:
The following will install the code, and process 3k events out of a TTbar relval

    cmsrel CMSSW_9_3_0_pre4
    git clone https://github.com/gpetruc/NanoAOD.git PhysicsTools/NanoAOD
    scram b -j 4
    cd PhysicsTools/NanoAOD/test
    cmsRun nano_cfg.py
    
