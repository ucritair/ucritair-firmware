cd build
if [[ "$OSTYPE" == "darwin"* ]]; then
	cmake .. -DMACOS=1 -DREBUILD_ATLAS=1
else
	cmake .. -DREBUILD_ATLAS=1
fi
make -j8

cd ..
build/app | tee ../script/atlasdata.txt

cd ../build
cmake .. -DDEBUG=1 -DBOARD=cat5340/nrf5340/cpuapp
make -j8
