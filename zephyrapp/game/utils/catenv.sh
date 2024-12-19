if [ -d ~/Documents/GitHub/zephyrproject/ ]; then
	source ~/Documents/GitHub/zephyrproject/zephyr/zephyr-env.sh
fi
if [ -z ${ZEPHYR_BASE} ]; then
	echo ZEPHYR_BASE is not set. Please source the zephyr shell environment
	return 1
fi
source $ZEPHYR_BASE/../venv/bin/activate

pip3 install pygame
pip3 install pillow
pip3 install PyOpenGL
pip3 install glfw
pip3 install imgui-bundle
pip3 install playsound3
