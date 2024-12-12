if [ ! -f meshes/mesh_assets.c ]; then
	utils/meshgen.py meshes
fi
if [ ! -f sounds/sound_assets.c ]; then
	utils/soundgen.py sounds
fi
cd build
cmake .. -DMACOS=1 -DREBUILD_ATLAS=0 -DDEBUG=1
make -j8
cd ..
build/app
