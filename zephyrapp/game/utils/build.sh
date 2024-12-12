cd build
make -j8
cd ..
build/app | tee ../atlasdata.txt
if [[ $1 == "--embedded" ]]; then
	cd ../build
	make -j8
fi

