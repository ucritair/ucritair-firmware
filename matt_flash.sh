#!/bin/bash
nrfjprog --recover --coprocessor CP_NETWORK
nrfjprog --recover
nrfjprog --program ble_hci.hex --sectorerase --verify --coprocessor CP_NETWORK
nrfjprog --program mcuboot.hex --sectorerase --verify
nrfjprog --reset
sleep 4
dfu-util --reset --download app.signed.bin
nrfjprog --reset
sleep 4
python3 load_w25q.py nrf70.bin $(ls /dev/tty.usbmodem* | tail -n 1) --skip-flush
