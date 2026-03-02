#!/usr/bin/env python3
"""
Trigger DFU mode, reboot, or set time on a uCrit device via USB serial.

Sends magic sequences over the USB CDC ACM serial port.
The firmware polls for these sequences and acts on them.

Requires: pip install pyserial

Usage:
    python3 trigger_dfu.py                      # Enter DFU mode
    python3 trigger_dfu.py --reboot             # Warm reboot (no DFU)
    python3 trigger_dfu.py --set-time           # Set device clock to host time
    python3 trigger_dfu.py --port /dev/ttyACM0  # Specify serial port
"""

import argparse
import calendar
import glob
import struct
import sys
import time
from datetime import datetime


DFU_MAGIC = bytes([0xCA, 0x7D, 0xF0, 0x01])
REBOOT_MAGIC = bytes([0xCA, 0x7D, 0xBE, 0x01])
SET_TIME_HEADER = bytes([0xCA, 0x7D, 0x54, 0x04])


def find_serial_port():
    """Auto-detect the uCrit serial port."""
    for pattern in ["/dev/tty.usbmodem*", "/dev/ttyACM*", "/dev/ttyUSB*"]:
        ports = sorted(glob.glob(pattern))
        if ports:
            return ports[0]
    return None


def main():
    parser = argparse.ArgumentParser(
        description="Trigger DFU mode, reboot, or set time on uCrit via USB serial"
    )
    group = parser.add_mutually_exclusive_group()
    group.add_argument(
        "--reboot",
        action="store_true",
        help="Trigger warm reboot (preserves RTC, no DFU)",
    )
    group.add_argument(
        "--set-time",
        action="store_true",
        help="Set device clock to current host time",
    )
    parser.add_argument(
        "--port",
        type=str,
        default=None,
        help="Serial port (auto-detected if omitted)",
    )
    args = parser.parse_args()

    port = args.port or find_serial_port()
    if not port:
        print("ERROR: No serial port found. Is the device connected via USB?")
        sys.exit(1)

    try:
        import serial
    except ImportError:
        print("ERROR: pyserial not installed (pip install pyserial)")
        sys.exit(1)

    print(f"Port: {port}")

    if args.set_time:
        # Send local time as a "fake UTC" timestamp — the device has no
        # timezone support and displays the raw value directly.
        now = calendar.timegm(time.localtime())
        msg = SET_TIME_HEADER + struct.pack("<I", now)
        dt = datetime.now().strftime("%Y-%m-%d %H:%M:%S %Z")
        print(f"Setting time to {dt} ({now})...")
        try:
            ser = serial.Serial(port, 115200, timeout=1)
            time.sleep(0.5)
            ser.write(msg)
            ser.flush()
            time.sleep(0.1)
            ser.close()
            print("Done! Device time set.")
        except Exception as e:
            print(f"Error: {e}")
            sys.exit(1)
    else:
        if args.reboot:
            magic = REBOOT_MAGIC
            action = "warm reboot"
        else:
            magic = DFU_MAGIC
            action = "DFU mode"

        print(f"Triggering {action}...")
        try:
            ser = serial.Serial(port, 115200, timeout=1)
            time.sleep(0.5)
            ser.write(magic)
            ser.flush()
            time.sleep(0.1)
            ser.close()
            print(f"Done! Device should be entering {action}.")
        except Exception as e:
            # Device disconnecting during write is expected (it rebooted)
            if "configured" in str(e).lower() or "disconnected" in str(e).lower():
                print(f"Done! Device rebooted into {action}.")
            else:
                print(f"Error: {e}")
                sys.exit(1)


if __name__ == "__main__":
    main()
