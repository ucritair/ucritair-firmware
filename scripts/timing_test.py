#!/usr/bin/env python3
"""
Boot timing test: reboot the device N times and measure full reboot cycle time.

Measures WALL-CLOCK time from triggering a USB serial warm reboot to the first
serial output (BOOT_TIMING line), which includes the MCUboot bootloader time.
Also captures k_uptime (app-only time) for comparison.

Requires: pip install pyserial

Usage:
    python3 timing_test.py              # 30 reboots (default)
    python3 timing_test.py -n 10        # 10 reboots
    python3 timing_test.py --port /dev/ttyACM0
"""

import argparse
import glob
import re
import sys
import time
import statistics


REBOOT_MAGIC = bytes([0xCA, 0x7D, 0xBE, 0x01])


def find_serial_port():
    for pattern in ["/dev/tty.usbmodem*", "/dev/ttyACM*", "/dev/ttyUSB*"]:
        ports = sorted(glob.glob(pattern))
        if ports:
            return ports[0]
    return None


def wait_for_boot_line(ser, timeout=30):
    """Wait for BOOT_TIMING line on serial. Returns k_uptime_ms or None."""
    start = time.time()
    while time.time() - start < timeout:
        line = ser.readline().decode("utf-8", errors="ignore").strip()
        if not line:
            continue
        m = re.search(r"BOOT_TIMING:\s*k_uptime=(\d+)\s*ms", line)
        if m:
            return int(m.group(1))
    return None


def wait_for_serial_port(preferred_port, max_wait=15):
    """Wait for a serial port to appear. Returns (serial_obj, port) or (None, None)."""
    import serial as pyserial
    for _ in range(max_wait):
        port = preferred_port or find_serial_port()
        if port:
            try:
                ser = pyserial.Serial(port, 115200, timeout=1)
                return ser, port
            except Exception:
                pass
        time.sleep(1)
    return None, None


def trigger_reboot(port):
    """Trigger warm reboot via USB serial. Returns wall-clock trigger timestamp."""
    import serial as pyserial
    ser = pyserial.Serial(port, 115200, timeout=1)
    time.sleep(0.3)
    ser.write(REBOOT_MAGIC)
    ser.flush()
    trigger_time = time.monotonic()
    time.sleep(0.1)
    try:
        ser.close()
    except Exception:
        pass
    return trigger_time


def main():
    parser = argparse.ArgumentParser(description="Boot timing test via USB serial reboot")
    parser.add_argument("-n", "--count", type=int, default=30, help="Number of reboots (default: 30)")
    parser.add_argument("--port", type=str, default=None, help="Serial port (auto-detected)")
    parser.add_argument("--boot-timeout", type=float, default=30.0, help="Seconds to wait for boot (default: 30)")
    args = parser.parse_args()

    import serial as pyserial

    port = args.port or find_serial_port()
    if not port:
        print("ERROR: No serial port found")
        sys.exit(1)

    print(f"Serial port: {port}")
    print(f"Will perform {args.count} reboot cycles")
    print(f"Measuring: wall-clock reboot->first_serial_output (includes MCUboot)\n")

    # Wait for device to be ready
    print("Waiting for device to be ready...")
    ser = pyserial.Serial(port, 115200, timeout=1)
    boot_ms = wait_for_boot_line(ser, timeout=args.boot_timeout)
    if boot_ms is not None:
        print(f"Initial k_uptime: {boot_ms} ms")
    ser.close()
    print()

    wall_times = []
    kuptime_times = []

    for i in range(args.count):
        print(f"[{i+1}/{args.count}] ", end="", flush=True)

        # Trigger reboot
        try:
            trigger_time = trigger_reboot(port)
        except Exception as e:
            print(f"SKIP - serial error: {e}")
            time.sleep(3)
            port = args.port or find_serial_port()
            continue

        # Wait for serial port to re-appear
        ser, port = wait_for_serial_port(args.port)
        if ser is None:
            print("SKIP - serial port did not reappear")
            continue

        # Read until BOOT_TIMING
        kuptime_ms = wait_for_boot_line(ser, timeout=args.boot_timeout)
        ser.close()

        if kuptime_ms is not None:
            wall_ms = int((time.monotonic() - trigger_time) * 1000)
            wall_times.append(wall_ms)
            kuptime_times.append(kuptime_ms)
            print(f"wall={wall_ms} ms  k_uptime={kuptime_ms} ms")
        else:
            print("TIMEOUT waiting for BOOT_TIMING")

    # Report
    print(f"\n{'='*60}")
    print(f"BOOT TIMING RESULTS ({len(wall_times)} samples)")
    print(f"{'='*60}")

    if wall_times:
        print(f"\n  Wall-clock (reboot trigger -> first serial output):")
        print(f"    Min:    {min(wall_times)} ms")
        print(f"    Max:    {max(wall_times)} ms")
        print(f"    Mean:   {statistics.mean(wall_times):.1f} ms")
        print(f"    Median: {statistics.median(wall_times):.1f} ms")
        if len(wall_times) > 1:
            print(f"    Stdev:  {statistics.stdev(wall_times):.1f} ms")

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
