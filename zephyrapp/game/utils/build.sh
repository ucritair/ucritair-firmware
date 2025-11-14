embedded=false
aq_first=false
radio=false
wifi=false
clean=true
OPTIONS=""

for var in "$@"
do
	if [ $var == "--embedded" ]; then
		echo "[BUILD] Targeting embedded"
		embedded=true
	fi
	if [ $var == "--aq-first" ]; then
		echo "[BUILD] Prioritizing AQ"
		aq_first=true
		OPTIONS="${OPTIONS} -DAQ_FIRST=ON"
	fi
	if [ $var == "--radio" ]; then
		echo "[BUILD] Enabling radio"
		radio=true
		OPTIONS="${OPTIONS} -DRADIO=ON"
	fi
	if [ $var == "--wifi" ]; then
		echo "[BUILD] Enabling WiFi"
		wifi=true
		OPTIONS="${OPTIONS} -DWIFI=ON"
	fi
done

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
if $embedded ; then
	if [ ! -d ../build ]; then
		mkdir ../build
	fi
	if [ ! -f ../build/makefile ]; then
		cd ../build
		cmake .. -DBOARD=cat5340/nrf5340/cpuapp $OPTIONS
		cd ../game
	fi

	cd ../build
	make -j8
	cd ../game
else
	build/app
fi

