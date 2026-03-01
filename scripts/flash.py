#!/usr/bin/env python3
"""
Flash firmware to a uCrit device.

Flow:
  1. Read BLE address from USB serial (device identification)
  2. Find device via BLE and trigger DFU mode (GPREGRET + reboot)
  3. Wait for MCUboot USB DFU to enumerate
  4. Flash with dfu-util
  5. Verify boot via serial

On macOS, CoreBluetooth doesn't expose raw BLE addresses, so the serial
address is logged for reference but scanning falls back to UUID/name
(works fine with a single device nearby).

Requires: pip install bleak pyserial
Also requires: dfu-util (brew install dfu-util / apt install dfu-util)

Usage:
    python3 flash.py firmware.bin
    python3 flash.py firmware.bin --skip-verify
"""

import argparse
import asyncio
import glob
import os
import platform
import re
import shutil
import subprocess
import sys
import time

from bleak import BleakScanner, BleakClient

SERVICE_UUID = "fc7d4395-1019-49c4-a91b-7491ecc40000"
DFU_CONTROL_CHAR = "fc7d4395-1019-49c4-a91b-7491ecc40009"
DFU_MAGIC = bytes([0xCA, 0x7D, 0xF0, 0x01])

IS_MACOS = platform.system() == "Darwin"


def find_serial_port():
    """Auto-detect the uCrit serial port."""
    for pattern in ["/dev/tty.usbmodem*", "/dev/ttyACM*", "/dev/ttyUSB*"]:
        ports = sorted(glob.glob(pattern))
        if ports:
            return ports[0]
    return None


def read_ble_addr_from_serial(port, timeout=10):
    """Read BLE_ADDR from serial output. Returns the address string or None."""
    import serial

    try:
        ser = serial.Serial(port, 115200, timeout=1)
        start = time.time()
        while time.time() - start < timeout:
            line = ser.readline().decode("utf-8", errors="ignore").strip()
            if not line:
                continue
            m = re.search(r"BLE_ADDR:\s*([0-9A-Fa-f:]{17})", line)
            if m:
                addr = m.group(1).upper()
                ser.close()
                return addr
        ser.close()
    except Exception as e:
        print(f"  Serial error: {e}")
    return None


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


async def find_and_trigger_dfu(target_addr=None, ble_timeout=15.0):
    """Find the BLE device and trigger DFU mode.
    Returns True on success."""

    device = None

    # On Linux, try matching by raw BLE address from serial
    if target_addr and not IS_MACOS:
        print(f"  Scanning for address {target_addr}...")
        device = await BleakScanner.find_device_by_filter(
            lambda d, adv: d.address.upper() == target_addr.upper(),
            timeout=ble_timeout,
        )
        if device:
            print(f"  Matched by address!")

    if device is None:
        print("  Scanning for uCrit...")
        device = await BleakScanner.find_device_by_filter(
            lambda d, adv: SERVICE_UUID.lower()
            in [s.lower() for s in (adv.service_uuids or [])],
            timeout=ble_timeout,
        )

    if device is None:
        device = await BleakScanner.find_device_by_filter(
            lambda d, adv: d.name and "ucrit" in d.name.lower(),
            timeout=ble_timeout,
        )

    if device is None:
        print("  ERROR: Could not find device via BLE")
        return False

    print(f"  Found: {device.name} ({device.address})")

    try:
        async with BleakClient(device, timeout=20.0) as client:
            await client.write_gatt_char(DFU_CONTROL_CHAR, DFU_MAGIC, response=True)
            return True
    except Exception as e:
        if "disconnected" in str(e).lower():
            return True  # Device rebooted into MCUboot — success
        print(f"  BLE error: {e}")
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


def main():
    parser = argparse.ArgumentParser(description="Flash firmware to uCrit via BLE + DFU")
    parser.add_argument("firmware", help="Path to firmware .bin file")
    parser.add_argument("--port", type=str, default=None, help="Serial port (auto-detected)")
    parser.add_argument("--ble-timeout", type=float, default=15.0)
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

    # Step 1: Identify device via serial
    print("\n[1/4] Identifying device...")
    serial_port = args.port or find_serial_port()
    target_addr = None
    if serial_port:
        print(f"  Serial port: {serial_port}")
        print(f"  Reading BLE address (up to 10s)...")
        target_addr = read_ble_addr_from_serial(serial_port, timeout=10)
        if target_addr:
            print(f"  BLE address: {target_addr}")
        else:
            print(f"  No BLE_ADDR on serial, will scan by UUID/name")
    else:
        print("  No serial port found, will scan by UUID/name")

    # Step 2: Trigger DFU via BLE
    print("\n[2/4] Triggering DFU via BLE...")
    if not asyncio.run(find_and_trigger_dfu(target_addr, ble_timeout=args.ble_timeout)):
        print("ERROR: Could not trigger DFU")
        sys.exit(1)
    print("  DFU triggered!")

    # Step 3: Wait for MCUboot USB DFU and flash
    print("\n[3/4] Flashing...")
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
        print("\n[4/4] Skipped verification")
    else:
        print("\n[4/4] Verifying boot...")
        time.sleep(3)
        boot_ms = wait_for_boot(port_hint=args.port)
        if boot_ms is not None:
            print(f"  Booted OK! (k_uptime={boot_ms} ms)")
        else:
            print("  Could not verify (device may still have booted OK)")

    print(f"\nDone! Flashed {fw_name}")


if __name__ == "__main__":
    main()
