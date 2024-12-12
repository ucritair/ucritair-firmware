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
`build.sh` builds the game for desktop.
It also calls the asset generation scripts if asset files are not found where they are expected, but it does not rebuild the atlas.
`embedbuild.sh` builds the game for embedded, and does not call asset generation scripts but does rebuild the atlas.
This is clearly insane. An overhaul is coming soon.
## Flashing
`flash.py` polls for flash-able devices and automatically starts a flashing process for each found.
It will give you `dfu-util` errors that nobody understands.

# Cleanup
`clean.sh` deletes asset files and stray temp directories.