#!/usr/bin/env python3
"""
Flash firmware to a uCrit device via USB serial + DFU.

Flow:
  1. Find device serial port
  2. Send magic bytes over serial to trigger MCUboot DFU mode
  3. Wait for MCUboot USB DFU to enumerate and flash with dfu-util
  4. Verify boot via serial
  5. Set device clock to host time

Requires: pip install pyserial
Also requires: dfu-util (brew install dfu-util / apt install dfu-util)

Usage:
    python3 flash.py firmware.bin
    python3 flash.py firmware.bin --skip-verify
    python3 flash.py firmware.bin --port /dev/ttyACM0
"""

import argparse
import calendar
import glob
import os
import re
import shutil
import struct
import subprocess
import sys
import time
from datetime import datetime


DFU_MAGIC = bytes([0xCA, 0x7D, 0xF0, 0x01])
SET_TIME_HEADER = bytes([0xCA, 0x7D, 0x54, 0x04])


def find_serial_port():
    """Auto-detect the uCrit serial port."""
    for pattern in ["/dev/tty.usbmodem*", "/dev/ttyACM*", "/dev/ttyUSB*"]:
        ports = sorted(glob.glob(pattern))
        if ports:
            return ports[0]
    return None


def trigger_dfu_via_serial(port):
    """Send DFU magic bytes over serial to reboot into MCUboot."""
    import serial

    try:
        ser = serial.Serial(port, 115200, timeout=1)
        time.sleep(0.5)
        ser.write(DFU_MAGIC)
        ser.flush()
        time.sleep(0.1)
        ser.close()
        return True
    except Exception as e:
        print(f"  Serial error: {e}")
        return False


def dfu_device_present():
    """Check if MCUboot DFU USB device is visible to dfu-util."""
    try:
        result = subprocess.run(
            ["dfu-util", "--list"],
            capture_output=True, text=True, timeout=5
        )
        return "2fe3" in result.stdout
    except (subprocess.TimeoutExpired, FileNotFoundError):
        return False


def flash_dfu(firmware_path, timeout=180):
    """Flash firmware using dfu-util. Returns True on success."""
    size = os.path.getsize(firmware_path)
    name = os.path.basename(firmware_path)
    print(f"  Downloading {name} ({size:,} bytes)...")

    try:
        proc = subprocess.Popen(
            ["dfu-util", "--alt", "0", "--download", firmware_path, "--reset"],
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True
        )

        deadline = time.time() + timeout
        output_lines = []
        last_pct = -1

        while True:
            if time.time() > deadline:
                proc.kill()
                print(f"  Timed out after {timeout}s")
                return False

            line = proc.stdout.readline()
            if not line and proc.poll() is not None:
                break
            if line:
                output_lines.append(line)
                if "]" in line and "%" in line:
                    try:
                        pct = int(line.split("]")[1].strip().split("%")[0].strip())
                        if pct >= last_pct + 25:
                            last_pct = pct
                            print(f"    {pct}%")
                    except (ValueError, IndexError):
                        pass

        proc.wait()
        output = "".join(output_lines)

        if "Download done" in output:
            return True
        if proc.returncode == 74 and "Download" in output:
            return True  # LIBUSB_ERROR_IO — device rebooted after flash

        print(f"  dfu-util exited with code {proc.returncode}")
        for line in output_lines[-5:]:
            print(f"    {line.rstrip()}")
        return False

    except Exception as e:
        print(f"  Error: {e}")
        return False


def wait_for_boot(port_hint=None, timeout=15):
    """Wait for device to boot. Returns boot_ms or None."""
    try:
        import serial
    except ImportError:
        return None

    print(f"  Waiting for device to boot (up to {timeout}s)...")
    for _ in range(timeout):
        port = port_hint or find_serial_port()
        if port:
            try:
                ser = serial.Serial(port, 115200, timeout=1)
                start = time.time()
                while time.time() - start < 5:
                    line = ser.readline().decode("utf-8", errors="ignore").strip()
                    m = re.search(r"BOOT_TIMING:\s*k_uptime=(\d+)\s*ms", line)
                    if m:
                        ser.close()
                        return int(m.group(1))
                ser.close()
            except Exception:
                pass
        time.sleep(1)
    return None


def set_device_time(port):
    """Set device RTC to current local time via USB serial.

    The device has no timezone support — it displays the raw timestamp
    directly. We send local time as a 'fake UTC' timestamp so the
    device shows the correct local time.
    """
    import serial

    now = calendar.timegm(time.localtime())
    msg = SET_TIME_HEADER + struct.pack("<I", now)
    try:
        ser = serial.Serial(port, 115200, timeout=1)
        time.sleep(0.5)
        ser.write(msg)
        ser.flush()
        time.sleep(0.1)
        ser.close()
        return datetime.now().strftime("%Y-%m-%d %H:%M:%S %Z")
    except Exception:
        return None


def main():
    parser = argparse.ArgumentParser(description="Flash firmware to uCrit via USB serial + DFU")
    parser.add_argument("firmware", help="Path to firmware .bin file")
    parser.add_argument("--port", type=str, default=None, help="Serial port (auto-detected)")
    parser.add_argument("--skip-verify", action="store_true")
    args = parser.parse_args()

    if not os.path.isfile(args.firmware):
        print(f"ERROR: File not found: {args.firmware}")
        sys.exit(1)
    if not shutil.which("dfu-util"):
        print("ERROR: dfu-util not found (brew install dfu-util)")
        sys.exit(1)

    fw_name = os.path.basename(args.firmware)
    fw_size = os.path.getsize(args.firmware)
    print(f"Firmware: {fw_name} ({fw_size:,} bytes)")

    # Step 1: Find serial port
    print("\n[1/5] Finding device...")
    serial_port = args.port or find_serial_port()
    if not serial_port:
        print("  ERROR: No serial port found. Is the device connected via USB?")
        sys.exit(1)
    print(f"  Serial port: {serial_port}")

    # Step 2: Trigger DFU via serial magic bytes
    print("\n[2/5] Triggering DFU via USB serial...")
    if not trigger_dfu_via_serial(serial_port):
        print("ERROR: Could not send DFU trigger")
        sys.exit(1)
    print("  DFU magic sent!")

    # Step 3: Wait for MCUboot USB DFU and flash
    print("\n[3/5] Flashing...")
    print("  Waiting for MCUboot USB DFU...")
    for i in range(20):
        time.sleep(1)
        if dfu_device_present():
            print("  MCUboot DFU ready!")
            break
    else:
        print("ERROR: MCUboot USB DFU did not appear (20s)")
        print("  Manual fallback: hold SELECT+START+DOWN, press RESET,")
        print("  then: dfu-util --alt 0 --download <firmware.bin> --reset")
        sys.exit(1)

    if not flash_dfu(args.firmware):
        print("ERROR: Flash failed")
        sys.exit(1)
    print("  Flash complete!")

    # Step 4: Verify boot
    if args.skip_verify:
        print("\n[4/5] Skipped verification")
    else:
        print("\n[4/5] Verifying boot...")
        time.sleep(3)
        boot_ms = wait_for_boot(port_hint=args.port)
        if boot_ms is not None:
            print(f"  Booted OK! (k_uptime={boot_ms} ms)")
        else:
            print("  Could not verify (device may still have booted OK)")

    # Step 5: Set device time
    print("\n[5/5] Setting device time...")
    time.sleep(1)
    time_port = args.port or find_serial_port()
    if time_port:
        iso = set_device_time(time_port)
        if iso:
            print(f"  Time set to {iso}")
        else:
            print("  Could not set time (device may need manual time sync)")
    else:
        print("  No serial port — skipping time set")

    print(f"\nDone! Flashed {fw_name}")


if __name__ == "__main__":
    main()
