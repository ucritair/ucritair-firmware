utils/meshgen.py meshes
utils/soundgen.py sounds
cd build
cmake .. -DMACOS=1 -DREBUILD_ATLAS=0 -DDEBUG=1
make -j8
cd ..
build/app
