/bin/rm build/makefile
/bin/rm ../build/makefile

/bin/rm meshes/mesh_assets.*
/bin/rm sounds/sound_assets.*
/bin/rm sounds/sprite_assets.*
/bin/rm data/*.c data/*.h
/bin/rm -r sounds/temp/

./utils/meshgen.py
./utils/soundgen.py
./utils/spritegen.py
./utils/itemgen.py
./utils/themegen.py
./utils/configgen.py

/bin/rm save.dat
/bin/rm sleep.dat
