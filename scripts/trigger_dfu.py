#!/usr/bin/env python3
"""
Trigger DFU mode or bootloader update on a uCritAir device via BLE.

Requires: pip install bleak

Usage:
    python3 trigger_dfu.py              # Enter DFU mode (for firmware flashing)
    python3 trigger_dfu.py --bl-update  # Trigger bootloader self-update
    python3 trigger_dfu.py --timeout 30 # Custom scan timeout
"""

import argparse
import asyncio
from bleak import BleakScanner, BleakClient

SERVICE_UUID = "fc7d4395-1019-49c4-a91b-7491ecc40000"
DFU_CONTROL_CHAR = "fc7d4395-1019-49c4-a91b-7491ecc40009"
BL_UPDATE_CHAR = "fc7d4395-1019-49c4-a91b-7491ecc4000a"
DFU_MAGIC = bytes([0xCA, 0x7D, 0xF0, 0x01])
BL_UPDATE_MAGIC = bytes([0xCA, 0x7D, 0xB0, 0x07])


async def main(args):
    print("Scanning for uCritAir...")
    device = await BleakScanner.find_device_by_filter(
        lambda d, adv: SERVICE_UUID.lower()
        in [s.lower() for s in (adv.service_uuids or [])],
        timeout=args.timeout,
    )
    if device is None:
        print("Service UUID not in advertisement, trying by name...")
        device = await BleakScanner.find_device_by_filter(
            lambda d, adv: d.name and "ucritair" in d.name.lower(),
            timeout=args.timeout,
        )
    if device is None:
        print("ERROR: Could not find uCritAir device!")
        return False

    print(f"Found: {device.name} ({device.address})")
    print("Connecting...")

    if args.bl_update:
        char_uuid = BL_UPDATE_CHAR
        magic = BL_UPDATE_MAGIC
        action = "bootloader self-update"
    else:
        char_uuid = DFU_CONTROL_CHAR
        magic = DFU_MAGIC
        action = "DFU mode"

    try:
        async with BleakClient(device, timeout=20.0) as client:
            print(f"Connected! MTU={client.mtu_size}")
            print(f"Triggering {action}...")
            await client.write_gatt_char(char_uuid, magic, response=True)
            print(f"Success! Device is entering {action}.")
    except Exception as e:
        # Device reboots immediately after receiving the command,
        # which drops the BLE connection. This is expected.
        if "disconnected" in str(e).lower():
            print(f"Device rebooted (connection dropped). {action} triggered successfully.")
        else:
            print(f"Error: {e}")
            return False

    return True


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Trigger DFU mode or bootloader update on uCritAir via BLE"
    )
    parser.add_argument(
        "--bl-update",
        action="store_true",
        help="Trigger bootloader self-update instead of DFU mode",
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=15.0,
        help="BLE scan timeout in seconds (default: 15)",
    )
    args = parser.parse_args()
    result = asyncio.run(main(args))
    exit(0 if result else 1)
