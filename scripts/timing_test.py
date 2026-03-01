#!/usr/bin/env python3
"""
Boot timing test: reboot the device N times and measure full reboot cycle time.

Measures WALL-CLOCK time from triggering a BLE warm reboot to the first serial
output (BOOT_TIMING line), which includes the MCUboot bootloader validation time.
Also captures k_uptime (app-only time) for comparison.

Requires: pip install bleak pyserial

Usage:
    python3 timing_test.py              # 30 reboots (default)
    python3 timing_test.py -n 10        # 10 reboots
    python3 timing_test.py --port /dev/ttyACM0
"""

import argparse
import asyncio
import glob
import re
import sys
import time
import statistics

from bleak import BleakScanner, BleakClient

SERVICE_UUID = "fc7d4395-1019-49c4-a91b-7491ecc40000"
DFU_CONTROL_CHAR = "fc7d4395-1019-49c4-a91b-7491ecc40009"
REBOOT_MAGIC = bytes([0xCA, 0x7D, 0xBE, 0x01])


def find_serial_port():
    patterns = ["/dev/tty.usbmodem*", "/dev/ttyACM*", "/dev/ttyUSB*"]
    for pattern in patterns:
        ports = sorted(glob.glob(pattern))
        if ports:
            return ports[0]
    return None


def wait_for_boot_line(ser, timeout=30):
    """
    Wait for BOOT_TIMING line on serial.
    Returns k_uptime_ms or None on timeout.
    Also returns BLE_ADDR if seen.
    """
    boot_ms = None
    ble_addr = None
    start = time.time()

    while time.time() - start < timeout:
        line = ser.readline().decode("utf-8", errors="ignore").strip()
        if not line:
            continue

        m = re.search(r"BOOT_TIMING:\s*k_uptime=(\d+)\s*ms", line)
        if m:
            boot_ms = int(m.group(1))

        m = re.search(r"BLE_ADDR:\s*([0-9A-Fa-f:]{17})", line)
        if m:
            ble_addr = m.group(1).upper()

        # Return as soon as we see BOOT_TIMING (don't wait for BLE_ADDR)
        if boot_ms is not None:
            return boot_ms, ble_addr

    return boot_ms, ble_addr


def wait_for_serial_port(preferred_port, max_wait=15):
    """Wait for a serial port to appear. Returns port path or None."""
    for _ in range(max_wait):
        port = preferred_port or find_serial_port()
        if port:
            try:
                import serial as pyserial
                ser = pyserial.Serial(port, 115200, timeout=1)
                return ser, port
            except Exception:
                pass
        time.sleep(1)
    return None, None


async def trigger_reboot_and_timestamp(ble_addr, timeout=15):
    """
    Trigger warm reboot via BLE. Returns the wall-clock timestamp of when
    the write completed (the reboot trigger moment), or None on failure.
    """
    device = None
    if ble_addr:
        device = await BleakScanner.find_device_by_filter(
            lambda d, adv: d.address.upper() == ble_addr.upper(),
            timeout=timeout,
        )
    if device is None:
        device = await BleakScanner.find_device_by_filter(
            lambda d, adv: SERVICE_UUID.lower()
            in [s.lower() for s in (adv.service_uuids or [])],
            timeout=timeout,
        )
    if device is None:
        device = await BleakScanner.find_device_by_filter(
            lambda d, adv: d.name and "ucrit" in d.name.lower(),
            timeout=timeout,
        )
    if device is None:
        return None

    try:
        async with BleakClient(device, timeout=20.0) as client:
            await client.write_gatt_char(DFU_CONTROL_CHAR, REBOOT_MAGIC, response=True)
            trigger_time = time.monotonic()
            return trigger_time
    except Exception as e:
        if "disconnected" in str(e).lower():
            # The reboot happened during/right after the write — use now as trigger time
            return time.monotonic()
        else:
            return None


def main():
    parser = argparse.ArgumentParser(description="Boot timing test via BLE reboot")
    parser.add_argument("-n", "--count", type=int, default=30, help="Number of reboots (default: 30)")
    parser.add_argument("--port", type=str, default=None, help="Serial port (auto-detected)")
    parser.add_argument("--timeout", type=float, default=15.0, help="BLE scan timeout (default: 15)")
    parser.add_argument("--boot-timeout", type=float, default=30.0, help="Seconds to wait for boot (default: 30)")
    args = parser.parse_args()

    import serial as pyserial

    port = args.port or find_serial_port()
    if not port:
        print("ERROR: No serial port found")
        sys.exit(1)

    print(f"Serial port: {port}")
    print(f"Will perform {args.count} reboot cycles")
    print(f"Measuring: wall-clock reboot→first_serial_output (includes MCUboot)\n")

    ser = pyserial.Serial(port, 115200, timeout=1)

    # Step 1: Wait for device to be booted and find BLE_ADDR
    print("Waiting for device to be ready...")
    boot_ms, ble_addr = wait_for_boot_line(ser, timeout=args.boot_timeout)
    if ble_addr:
        print(f"Device BLE address: {ble_addr}")
    else:
        print("No BLE_ADDR on serial. Will scan by UUID/name.")
    if boot_ms is not None:
        print(f"Initial k_uptime: {boot_ms} ms")
    print()

    wall_times = []
    kuptime_times = []

    # Step 2: Reboot loop
    for i in range(args.count):
        print(f"[{i+1}/{args.count}] ", end="", flush=True)

        # Close serial before triggering reboot (USB will disappear)
        ser.close()

        # Trigger reboot and record the exact moment
        trigger_time = asyncio.run(trigger_reboot_and_timestamp(ble_addr, timeout=args.timeout))
        if trigger_time is None:
            print("SKIP - BLE device not found")
            # Re-open serial for next attempt
            ser, port = wait_for_serial_port(args.port)
            if ser is None:
                print("  FATAL: Cannot reopen serial port")
                break
            # Drain and find BLE_ADDR again
            _, ble_addr_new = wait_for_boot_line(ser, timeout=args.boot_timeout)
            if ble_addr_new:
                ble_addr = ble_addr_new
            continue

        # Wait for serial port to re-appear (USB re-enumerates after reboot)
        ser, port = wait_for_serial_port(args.port)
        if ser is None:
            print("SKIP - serial port did not reappear")
            continue

        # Read serial until we see BOOT_TIMING — measure wall time to this moment
        boot_start = time.monotonic()
        # We already have a serial connection; now wait for the BOOT_TIMING line
        kuptime_ms, addr_new = wait_for_boot_line(ser, timeout=args.boot_timeout)

        if kuptime_ms is not None:
            first_output_time = time.monotonic()
            wall_ms = int((first_output_time - trigger_time) * 1000)
            wall_times.append(wall_ms)
            kuptime_times.append(kuptime_ms)
            print(f"wall={wall_ms} ms  k_uptime={kuptime_ms} ms")
            if addr_new:
                ble_addr = addr_new
        else:
            print("TIMEOUT waiting for BOOT_TIMING")

    if ser and ser.is_open:
        ser.close()

    # Step 3: Report
    print(f"\n{'='*60}")
    print(f"BOOT TIMING RESULTS ({len(wall_times)} samples)")
    print(f"{'='*60}")

    if wall_times:
        print(f"\n  Wall-clock (reboot trigger → first serial output):")
        print(f"    Min:    {min(wall_times)} ms")
        print(f"    Max:    {max(wall_times)} ms")
        print(f"    Mean:   {statistics.mean(wall_times):.1f} ms")
        print(f"    Median: {statistics.median(wall_times):.1f} ms")
        if len(wall_times) > 1:
            print(f"    Stdev:  {statistics.stdev(wall_times):.1f} ms")
        print(f"    Range:  {max(wall_times) - min(wall_times)} ms")

        print(f"\n  k_uptime (app-only, after MCUboot):")
        print(f"    Min:    {min(kuptime_times)} ms")
        print(f"    Max:    {max(kuptime_times)} ms")
        print(f"    Mean:   {statistics.mean(kuptime_times):.1f} ms")

        print(f"\n  Estimated MCUboot overhead (wall - k_uptime):")
        overheads = [w - k for w, k in zip(wall_times, kuptime_times)]
        print(f"    Min:    {min(overheads)} ms")
        print(f"    Max:    {max(overheads)} ms")
        print(f"    Mean:   {statistics.mean(overheads):.1f} ms")
        if len(overheads) > 1:
            print(f"    Stdev:  {statistics.stdev(overheads):.1f} ms")

        print(f"\n  Wall times: {wall_times}")
    else:
        print("  No samples collected!")


if __name__ == "__main__":
    main()
