mkdir soundgen_temp
ffmpeg -i $1 -ac 1 -ar 32000 -c:a pcm_s16le -y soundgen_temp/out.wav
ffmpeg -i soundgen_temp/out.wav -y soundgen_temp/out.au
utils/encoder.out 23 < soundgen_temp/out.au > soundgen_temp/out.bin
python3 utils/soundgen.py soundgen_temp/out.bin
