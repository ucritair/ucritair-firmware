# uCritAir Firmware

Initial Public Release 

## Build instructions

Setup:
 - Install `west` and an appropriate toolchain (`west sdk install`, we're on `zephyr-sdk-0.16.8`) . Guide : https://docs.zephyrproject.org/latest/develop/getting_started/index.html
 - You may need to checkout zephyr to 03959a20 : https://github.com/zephyrproject-rtos/zephyr/commit/03959a20f7834ffe50a51df2b2e45ef87d1b610b
 - Clone this repo as `cat_software`
 - `west init -l cat_software`
 - `west update`
 - `(cd zephyr ; git apply ../cat_software/zephyr.patch)`
 - `export ZEPHYR_BASE=$(realpath zephyr)`
 - `cd cat_software/zephyrapp`
 - `mkdir build`
 - `cmake .. -DBOARD=cat5340/nrf5340/cpuapp`

Then to build and flash:
 - `make -j8 && west sign --tool imgtool && sudo dfu-util --download zephyr/zephyr.signed.bin`
