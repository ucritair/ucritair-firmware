embedded=false
aq_first=false
radio=false
wifi=false
clean=false
OPTIONS=""

for var in "$@"
do
	if [ $var == "--embedded" ]; then
		embedded=true
	fi
	if [ $var == "--aq-first" ]; then
		aq_first=true
		OPTIONS="${OPTIONS} -DAQ_FIRST=ON"
	fi
	if [ $var == "--radio" ]; then
		radio=true
		OPTIONS="${OPTIONS} -DRADIO=ON"
	fi
	if [ $var == "--wifi" ]; then
		wifi=true
		OPTIONS="${OPTIONS} -DWIFI=ON"
	fi
	if [ $var == "--clean" ]; then
		clean=true
	fi
done

if $clean; then
	trash ../build
	trash build
	trash utils/cache
	trash assets
fi

# DESKTOP

# INIT DIRS
if [ ! -d build ]; then
	mkdir build
fi
if [ ! -d utils/cache ]; then
	mkdir utils/cache
fi
if [ ! -d assets ]; then
	mkdir assets
fi

# INIT FILES
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

	if $wifi; then
		cp ../build/zephyr/zephyr.signed.bin utils/cache/wifi.bin
		echo "[BUILD] Cached WiFi build"
	elif $radio; then
		cp ../build/zephyr/zephyr.signed.bin utils/cache/radio.bin
		echo "[BUILD] Cached radio build"
	else
		cp ../build/zephyr/zephyr.signed.bin utils/cache/standard.bin
		echo "[BUILD] Cached standard build"
	fi
	cp ../build/zephyr/zephyr.signed.bin utils/cache/latest.bin
	echo "[BUILD] Cached latest build"
fi

if ! $embedded; then
	build/app
fi

