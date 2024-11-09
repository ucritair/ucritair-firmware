import serial, sys, time, struct

fw_path = sys.argv[1]

with open(fw_path, 'rb') as fd:
	fw_blob = fd.read()

persist_offset = 0x1000 + len(fw_blob)
persist_offset += 0x1000 - (persist_offset%0x1000)

header = struct.pack('<IIII', 0xeebeefee, 0x1000, len(fw_blob), persist_offset + 0x10000)

header += b'\0' * (0x1000-len(header))

write_blob = header + fw_blob

write_blob += b'\0' * (0x1000 - (len(write_blob)%0x1000))

buf = b''
def wait_for(s):
	global buf
	print('Wait for', s)
	while True:
		r = port.read(port.in_waiting)
		buf += r
		if s in buf:
			buf = buf[buf.index(s)+len(s):]
			break
		elif r:
			print(r)

while True:
	try:
		port = serial.Serial(sys.argv[2], baudrate=115200)
		break
	except:
		pass

if '--skip-flush' not in sys.argv:
	print("Connected, flush...")

	wait_for(b"CAT_init")
	wait_for(b'uart:~$')

print("Erase 0 ", hex(len(write_blob)))
port.write(b"flash erase w25q128@1 0 "+hex(len(write_blob)).encode('ascii')+b"\n")

wait_for(b'Erase success')

port.write(b"flash load w25q128@1 0 "+hex(len(write_blob)).encode('ascii')+b"\n")

wait_for(b'Loading...')

offset = 0
while write_blob:
	w = write_blob[:0x80]
	offset += len(w)
	port.write(w)
	write_blob = write_blob[0x80:]
	# wait_for(b'Written')
	r = port.read(port.in_waiting)
	print('>', r)
	if b'buffer full' in r:
		raise ValueError("fuck")
	print(hex(len(write_blob)), 'b left')
	time.sleep(0.02)

time.sleep(1)
print('>', port.read(port.in_waiting))