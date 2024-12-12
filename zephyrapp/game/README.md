# Notices
- Utilities should always be run from the `game` directory.
- `flash.py` may require you to enter your password before it can work.

# Setup
## Environment
For many of the critical utilities to work, the following must be sourced:
- `zephyrproject/venv/bin/activate`
- `zephyrproject/zephyr/zephyr-env.sh`
- `zephyrapp/game/utils/catenv/bin/activate`
`envgen.sh` generates `catenv`, activates it, and silently installs requisite python packages.
It also sources the zephyr scripts IFF they are found at the location where I keep them on my machine.
If you keep them at some other location, you will need to source them yourself.
It should itself be sourced, to properly activate the `catenv` virtual environment.

# Building
## Assets
The scripts `meshgen.py`, `soundgen.py`, and `spritegen.py` are used to generate asset files.
## Application
`build.sh` builds the game, including asset files.
To build for embedded, pass the `--embedded` flag.
## Flashing
`flash.py` polls for flash-able devices and automatically starts a flashing process for each found.
It will give you `dfu-util` errors that nobody understands.

# Cleanup
`clean.sh` deletes asset files and stray temp directories.

# Complete Example
In `cat_software/zephyrapp/game/`, with a CAT in bootloader mode connected by USB:
```
. utils/envgen.sh
./utils/build.sh --embedded
./utils/flash.py
```