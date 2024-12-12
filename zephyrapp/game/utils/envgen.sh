if [ -d ~/Documents/GitHub/zephyrproject ]; then
	source ~/Documents/GitHub/zephyrproject/venv/bin/activate
	source ~/Documents/GitHub/zephyrproject/zephyr/zephyr-env.sh
fi
if [ ! -d utils/catenv ]; then
	python -m venv utils/catenv
fi
source utils/catenv/bin/activate
pip install pygame --quiet
pip install pillow --quiet

