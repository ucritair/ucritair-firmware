# DESKTOP
if [ ! -d build ]; then
	mkdir build
fi
if [ ! -f build/makefile ]; then
	cd build
	cmake ..
	cd ..
fi

cd build
make
cd ..

# EMBEDDED
if [[ $1 == "--embedded" ]]; then
	if [ ! -d ../build ]; then
		mkdir ../build
	fi
	if [ ! -f ../build/makefile ]; then
		cd ../build
		cmake .. -DBOARD=cat5340/nrf5340/cpuapp
		cd ../game
	fi

	cd ../build
	if [[ $2 == "--aq-first" ]]; then
		make -j8 CFLAGS="-DAQ_FIRST=ON"
	else
		make -j8
	fi
	cd ../game
else
	if [[ $1 == "--clean" ]]; then
		trash *.dat
		trash persist/*.dat
	fi
	build/app
fi

