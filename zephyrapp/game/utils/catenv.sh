if [ -d ~/Documents/GitHub/zephyrproject/ ]; then
	source ~/Documents/GitHub/zephyrproject/zephyr/zephyr-env.sh
fi
if [ -z ${ZEPHYR_BASE} ]; then
	echo ZEPHYR_BASE is not set. Please source the zephyr shell environment
	return 1
fi
source $ZEPHYR_BASE/../venv/bin/activate

pip install --upgrade pip

pip install pillow
pip install PyOpenGL
pip install glfw
pip install imgui-bundle
pip install playsound3
pip install numpy
pip install qrcode
pip install meshtastic
pip install protobuf grpcio-tools