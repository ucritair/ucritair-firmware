# Zephyr Environment
To activate the shell environment:
`source (...)/zephyrproject/zephyr/zephyr-env.sh`
Verify that all is well:
`echo $ZEPHYR_BASE`
To activate the python virtual environment:
`source $ZEPHYR_BASE/../venv/bin/activate`

# Building
In `zephyrapp/`...
Make directory `build/` and go to it
Run `cmake` if `build/` is fresh:
`cmake .. -DBOARD=cat5340/nrf5340/cpuapp`
Finally:
`make -j8`

# Signing
Still in `zephyrapp/build/`...
`west sign --tool imgtool`

# Flashing
Connect device via USB
Use `dfu-util --list` to find its serial number.
Stil in `zephyrapp/build/`...
`dfu-util --serial=<SERIAL NUMBER> --download zephyr/zephyr.signed.bin --reset`

# Launching the editor
In `zephyrapp/game/`...
To ensure that python virtual environment has all packages needed by the editor:
`source utils/catenv.sh`
To launch the editor itself:
`editor/editor.py`