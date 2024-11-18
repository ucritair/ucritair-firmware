import sys;
import os;
import pathlib as pl;
import multiprocessing as mp;


github_dir = os.path.join(pl.Path.home(), "Documents/GitHub");
zproj_dir = os.path.join(github_dir, "zephyrproject");
zapp_dir = os.path.join(github_dir, "cat_software_embd/zephyrapp");
game_dir = os.path.join(zapp_dir, "game");
build_dir = os.path.join(zapp_dir, "build");

def flash_proc(serial):
    print(f"Flashing game to {serial}...");
    os.chdir(build_dir);
    os.popen(f"west sign --tool imgtool && sudo dfu-util --serial={serial} --download zephyr/zephyr.signed.bin --reset").read();
    print("Done!");

if __name__ == '__main__':
    mp.freeze_support();
    
    print("Building game...");
    os.chdir(game_dir);
    os.popen("(cd build; make -j8) && build/app | tee ../script/atlasdata.txt").read();
    print("Done!");
    print("Building as zephyr app...");
    os.chdir(build_dir);
    os.popen("make").read();
    print("Done!");

    print("Finding serials...");
    dfu_output = os.popen("dfu-util --list").read();
    dfu_tokens = dfu_output.split();
    dfu_tokens = [t for t in dfu_tokens if "serial=" in t];
    def strip_serial(token):
        token = token.split("=")[1];
        token = token.replace("\"", "");
        return token;
    dfu_serials = [strip_serial(t) for t in dfu_tokens];
    print("Found: ", end="");
    print(dfu_serials);

    procs = [mp.Process(target=flash_proc, args=(serial,)) for serial in dfu_serials];
    for proc in procs:
        proc.start();
    for proc in procs:
        proc.join();

    print("All complete. Exiting!");
