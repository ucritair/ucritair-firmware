cd build
echo $PWD
cmake .. -DMACOS=1 -DREBUILD_ATLAS=0 -DDEBUG=1
make -j8
cd ..
echo $PWD
build/app
