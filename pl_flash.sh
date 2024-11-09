set -exuo

cd /home/louis/Documents/ee/cat/zephyrproject/zephyr/build_ble_hci
make -j8 flash

cd /home/louis/Documents/ee/cat/cat_software/mcuboot/build
make -j8 flash

nrfjprog --reset
sleep 4

cd /home/louis/Documents/ee/cat/cat_software/zephyrapp/build
sudo dfu-util --reset --download zephyr/zephyr.signed.bin

nrfjprog --reset

cd /home/louis/Documents/ee/cat/cat_software/zephyrapp/script
python3 load_w25q.py /home/louis/Documents/ee/cat/zephyrproject/modules/hal/nordic/zephyr/blobs/wifi_fw_bins/default/nrf70.bin /dev/ttyACM1

nrfjprog --reset
