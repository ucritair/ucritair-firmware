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
make -j8
cd ..

# EMBEDDED
if [[ $1 == "--embedded" ]]; then
	if [ ! -d ../build ]; then
		mkdir ../build
	fi
	if [ ! -f ../build/makefile ]; then
		cd ../build
		cmake .. -DBOARD=cat5340/nrf5340/cpuapp
		cd ..
	fi

	cd build
	make -j8
else
	build/app
fi

