cd ~/Documents/GitHub/cat_software/zephyrapp/game/build
cmake .. -DMACOS=1 -DREBUILD_ATLAS=0
make -j8
cd ~/Documents/GitHub/cat_software/zephyrapp/game
build/app
