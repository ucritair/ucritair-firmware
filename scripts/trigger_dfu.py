#!/usr/bin/env python3
"""
Trigger DFU mode, reboot, or bootloader update on a uCrit device via BLE.

Can auto-detect which BLE device to target by reading the BLE_ADDR from
the USB serial port (matches the device physically connected to this computer).

Requires: pip install bleak pyserial

Usage:
    python3 trigger_dfu.py                      # Auto-detect via serial, enter DFU
    python3 trigger_dfu.py --bl-update           # Trigger bootloader self-update
    python3 trigger_dfu.py --reboot              # Warm reboot (no DFU)
    python3 trigger_dfu.py --address XX:XX:...   # Target a specific BLE address
    python3 trigger_dfu.py --port /dev/ttyACM0   # Specify serial port
    python3 trigger_dfu.py --no-serial           # Skip serial, scan by UUID/name
"""

import argparse
import asyncio
import glob
import re
import sys
import time

from bleak import BleakScanner, BleakClient

SERVICE_UUID = "fc7d4395-1019-49c4-a91b-7491ecc40000"
DFU_CONTROL_CHAR = "fc7d4395-1019-49c4-a91b-7491ecc40009"
BL_UPDATE_CHAR = "fc7d4395-1019-49c4-a91b-7491ecc4000a"
DFU_MAGIC = bytes([0xCA, 0x7D, 0xF0, 0x01])
REBOOT_MAGIC = bytes([0xCA, 0x7D, 0xBE, 0x01])
BL_UPDATE_MAGIC = bytes([0xCA, 0x7D, 0xB0, 0x07])


def find_serial_port():
    """Auto-detect the uCrit serial port."""
    patterns = [
        "/dev/tty.usbmodem*",
        "/dev/ttyACM*",
        "/dev/ttyUSB*",
    ]
    for pattern in patterns:
        ports = sorted(glob.glob(pattern))
        if ports:
            return ports[0]
    return None


def read_ble_addr_from_serial(port, timeout=10):
    """Read BLE_ADDR from serial output. Returns the address string or None."""
    try:
        import serial
    except ImportError:
        print("pyserial not installed (pip install pyserial), skipping serial detection")
        return None

    print(f"Reading BLE address from {port} (waiting up to {timeout}s)...")
    try:
        ser = serial.Serial(port, 115200, timeout=1)
        start = time.time()
        while time.time() - start < timeout:
            line = ser.readline().decode("utf-8", errors="ignore").strip()
            if not line:
                continue
            # Match "BLE_ADDR: XX:XX:XX:XX:XX:XX (type)"
            m = re.search(r"BLE_ADDR:\s*([0-9A-Fa-f:]{17})", line)
            if m:
                addr = m.group(1).upper()
                print(f"Detected BLE address: {addr}")
                ser.close()
                return addr
        ser.close()
        print(f"No BLE_ADDR seen on {port} within {timeout}s")
    except Exception as e:
        print(f"Serial error on {port}: {e}")
    return None


async def find_device(args, target_addr=None):
    """Find the BLE device, optionally filtering by address."""
    if target_addr:
        print(f"Scanning for device with address {target_addr}...")
        device = await BleakScanner.find_device_by_filter(
            lambda d, adv: d.address.upper() == target_addr.upper(),
            timeout=args.timeout,
        )
        if device:
            return device
        print(f"Address {target_addr} not found, falling back to UUID/name scan...")

    print("Scanning for uCrit...")
    device = await BleakScanner.find_device_by_filter(
        lambda d, adv: SERVICE_UUID.lower()
        in [s.lower() for s in (adv.service_uuids or [])],
        timeout=args.timeout,
    )
    if device is None:
        print("Service UUID not in advertisement, trying by name...")
        device = await BleakScanner.find_device_by_filter(
            lambda d, adv: d.name and "ucrit" in d.name.lower(),
            timeout=args.timeout,
        )
    return device


async def main(args):
    # Determine target BLE address
    target_addr = None
    if args.address:
        target_addr = args.address
    elif not args.no_serial:
        port = args.port or find_serial_port()
        if port:
            target_addr = read_ble_addr_from_serial(port, timeout=args.serial_timeout)
        else:
            print("No serial port found, scanning by UUID/name...")

    device = await find_device(args, target_addr)
    if device is None:
        print("ERROR: Could not find uCrit device!")
        return False

    print(f"Found: {device.name} ({device.address})")

    if args.reboot:
        char_uuid = DFU_CONTROL_CHAR
        magic = REBOOT_MAGIC
        action = "warm reboot"
    elif args.bl_update:
        char_uuid = BL_UPDATE_CHAR
        magic = BL_UPDATE_MAGIC
        action = "bootloader self-update"
    else:
        char_uuid = DFU_CONTROL_CHAR
        magic = DFU_MAGIC
        action = "DFU mode"

    print(f"Connecting to trigger {action}...")
    try:
        async with BleakClient(device, timeout=20.0) as client:
            print(f"Connected! MTU={client.mtu_size}")
            print(f"Triggering {action}...")
            await client.write_gatt_char(char_uuid, magic, response=True)
            print(f"Success! Device is entering {action}.")
    except Exception as e:
        if "disconnected" in str(e).lower():
            print(f"Device rebooted (connection dropped). {action} triggered successfully.")
        else:
            print(f"Error: {e}")
            return False

    return True


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Trigger DFU mode, reboot, or bootloader update on uCrit via BLE"
    )
    parser.add_argument(
        "--bl-update",
        action="store_true",
        help="Trigger bootloader self-update instead of DFU mode",
    )
    parser.add_argument(
        "--reboot",
        action="store_true",
        help="Trigger warm reboot (no DFU, preserves RTC)",
    )
    parser.add_argument(
        "--address",
        type=str,
        default=None,
        help="Target a specific BLE address (e.g., AA:BB:CC:DD:EE:FF)",
    )
    parser.add_argument(
        "--port",
        type=str,
        default=None,
        help="Serial port to read BLE address from (auto-detected if omitted)",
    )
    parser.add_argument(
        "--no-serial",
        action="store_true",
        help="Skip serial port detection, scan by UUID/name only",
    )
    parser.add_argument(
        "--serial-timeout",
        type=float,
        default=10.0,
        help="Seconds to wait for BLE_ADDR on serial (default: 10)",
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=15.0,
        help="BLE scan timeout in seconds (default: 15)",
    )
    args = parser.parse_args()
    result = asyncio.run(main(args))
    sys.exit(0 if result else 1)
