source ~/Documents/GitHub/zephyrproject/venv/bin/activate
source ~/Documents/GitHub/zephyrproject/zephyr/zephyr-env.sh

cd ~/Documents/GitHub/cat_software/zephyrapp/game/build
if [[ "$OSTYPE" == "darwin"* ]]; then
	cmake .. -DMACOS=1 -DREBUILD_ATLAS=1
else
	cmake .. -DREBUILD_ATLAS=1
fi
make -j8

cd ~/Documents/GitHub/cat_software/zephyrapp/game
build/app | tee ../script/atlasdata.txt

cd ~/Documents/GitHub/cat_software/zephyrapp/build
make
