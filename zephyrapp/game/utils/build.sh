embedded=false
aq_first=false
radio=false
wifi=false
clean=false
OPTIONS=""

while true; do
	case "$1" in
		--embedded ) embedded=true; shift ;;
		--aq-first )
			aq_first=true;
			OPTIONS="${OPTIONS} -DAQ_FIRST=ON";
			shift ;;
		--radio )
			radio=true;
			OPTIONS="${OPTIONS} -DRADIO=ON";
			shift ;;
		--wifi )
			wifi=true;
			OPTIONS="${OPTIONS} -DWIFI=ON";
			shift ;;
		--research_name )
			OPTIONS="${OPTIONS} -DRESEARCH_NAME=$2";
			shift ;;
		--clean ) clean=true; shift ;;
		-- ) shift; break ;;
		* ) break ;;
	esac
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
	cmake .. $OPTIONS
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

