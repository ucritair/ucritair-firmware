set -exuo

cd /home/louis/Documents/ee/cat/zephyrproject/zephyr/build_ble_hci
make -j8 flash

cd /home/louis/Documents/ee/cat/cat_software/mcuboot/build
make -j8 flash

nrfjprog --reset
sleep 4

cd /home/louis/Documents/ee/cat/cat_software/zephyrapp/build
sudo dfu-util --reset --download zephyr/zephyr.signed.bin