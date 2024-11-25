import sys;
import os;
import pathlib as pl;
import multiprocessing as mp;

github_dir = os.path.join(pl.Path.home(), "Documents/GitHub");
zproj_dir = os.path.join(github_dir, "zephyrproject");
zapp_dir = os.path.join(github_dir, "cat_software/zephyrapp");
game_dir = os.path.join(zapp_dir, "game");
build_dir = os.path.join(zapp_dir, "build");

def flash_proc(serial, serial_list):
    print(f"Flashing game to {serial}...");
    os.chdir(build_dir);
    os.popen(f"west sign --tool imgtool && sudo dfu-util --serial={serial} --download zephyr/zephyr.signed.bin --reset").read();
    serial_list.remove(serial);
    print("Done!");

if __name__ == '__main__':
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
        for serial in dfu_serials:
            if not serial in serial_list:
                print(f"Discovered {serial}");
                serial_list.append(serial);
                proc = mp.Process(target=flash_proc, args=(serial, serial_list));
                proc.start();
                proc_list.append(proc);
    for proc in proc_list:
        proc.join();
