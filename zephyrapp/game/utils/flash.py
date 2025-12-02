#!/usr/bin/env python3

import sys;
import os;
import pathlib as pl;
import multiprocessing as mp;

def flash_proc(image, serial, serial_list):
	print(f"Flashing {image} to {serial}...");
	os.popen(f"sudo dfu-util --serial={serial} --download {image} --reset").read();
	serial_list.remove(serial);
	print("Done!");

if __name__ == '__main__':

	args = sys.argv[1:];
	wifi = "--wifi" in args;
	radio = "--radio" in args;
	standard = "--standard" in args;

	image = "utils/build_cache/latest.bin"
	if wifi:
		image = "utils/build_cache/wifi.bin"
	elif radio:
		image = "utils/build_cache/radio.bin"
	elif standard:
		image = "utils/build_cache/standard.bin"

	try:
		mp.freeze_support();

		serial_list = [];
		proc_list = [];
		
		print("Polling DFU serials...");
		while True:
			dfu_output = os.popen("dfu-util --list").read();
			dfu_tokens = dfu_output.split();
			dfu_tokens = [t for t in dfu_tokens if "serial=" in t];
			
			def strip_serial(token):
				token = token.split("=")[1];
				token = token.replace("\"", "");
				return token;
			dfu_serials = [strip_serial(t) for t in dfu_tokens];
			dfu_serials = [t for t in dfu_serials if t != "UNKNOWN"];

			for serial in dfu_serials:
				if not serial in serial_list:
					print(f"Discovered {serial}");
					serial_list.append(serial);
					proc = mp.Process(target=flash_proc, args=(image, serial, serial_list));
					proc.start();
					proc_list.append(proc);
	
	except KeyboardInterrupt:
		for proc in proc_list:
			proc.join();
