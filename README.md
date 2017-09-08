# NanoAOD
Development repository for a new flat cms dataformat (will be merged in CMSSW once the prototyping is over)

## Current recipe:
The following will install the code, and process 3k events out of a TTbar relval

    cmsrel CMSSW_9_3_0_pre4
    cd CMSSW_9_3_0_pre4/src/
    cmsenv
    git clone https://github.com/gpetruc/NanoAOD.git PhysicsTools/NanoAOD
    scram b -j 4
    cd PhysicsTools/NanoAOD/test
    # requires grid proxy
    cmsRun nano_cfg.py

You should now two files: `nano.root` and `lzma.root`. The content is identical
but the latter is compressed using the LZMA.
