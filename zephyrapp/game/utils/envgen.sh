if [ -d ~/Documents/GitHub/zephyrproject ]; then
	source ~/Documents/GitHub/zephyrproject/zephyr/zephyr-env.sh
else
	echo IF YOU DIDN'T SOURCE THE ZEPHYR SHELL ENVIRONMENT YOU'RE COOKED!
fi
source $ZEPHYR_BASE/../venv/bin/activate
pip install pygame --quiet
pip install pillow --quiet

