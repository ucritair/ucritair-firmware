import os
import os.path
import subprocess
import sys

images = [
	'shortpathvac.png'
]

assets = os.path.dirname(__file__) + '/../assets/'
output = sys.argv[1]

def hex4(x):
	h = hex(x)[2:]
	h = (4-len(h))*'0' + h
	return '0x' + h

for image in images:
	w, h = [int(x) for x in subprocess.check_output(['identify', '-ping', '-format', "%w %h", f'{assets}/{image}']).strip().split()]
	print(f"{image}: {w}x{h}")
	# b = subprocess.check_output(['magick', f'{assets}/{image}', '-strip', '-define', 'bmp:subtype=RGB565', 'bmp2:-'])
	b = subprocess.check_output(['ffmpeg', '-vcodec', 'png', '-i', f'{assets}/{image}', '-vcodec', 'rawvideo', '-f', 'rawvideo', '-pix_fmt', 'rgb565', '-'])
	print(type(b), len(b), w*h*2)
	data = b[:w*h*2]
	with open(f"{output}/images.c", 'w') as fd:
		fd.write('#include <stdint.h>\n')
		fd.write(f'const int image_w = {w};\n')
		fd.write(f'const int image_h = {h};\n')
		fd.write("const uint16_t image_data[] = {\n\t")
		for i in range(len(data)//2):
			fd.write(hex4((data[(i*2)+0] << 8) | (data[(i*2)+1] << 0)) + ',')
			if (i%16) == 15:
				fd.write('\n\t')
		fd.write('\n};\n')