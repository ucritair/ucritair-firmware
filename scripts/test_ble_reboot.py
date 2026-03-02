#!/usr/bin/env python3
"""Test BLE address persistence across warm reboots.

Reads the BLE_ADDR from the device's serial console before and after
warm reboots. PASS = same address across reboots.
"""

import glob
import sys
import time

import serial

CMD_WARM_REBOOT = bytes([0xCA, 0x7D, 0xBE, 0x01])


def find_serial_port():
    patterns = ["/dev/tty.usbmodem*", "/dev/ttyACM*", "/dev/ttyUSB*"]
    for pat in patterns:
        ports = sorted(glob.glob(pat))
        if ports:
            return ports[0]
    return None


def read_boot_output(port_path, timeout=12):
    """Read serial output after boot, looking for BLE_ADDR and other info."""
    ble_addr = None
    lines = []
    try:
        with serial.Serial(port_path, 115200, timeout=1) as ser:
            ser.reset_input_buffer()
            start = time.time()
            while time.time() - start < timeout:
                line = ser.readline().decode("utf-8", errors="replace").strip()
                if line:
                    lines.append(line)
                    print(f"    {line}")
                    if "BLE_ADDR:" in line:
                        ble_addr = line.split("BLE_ADDR:")[1].strip()
                    # Stop once we see advertising started (boot complete)
                    if "Advertising successfully started" in line:
                        break
    except Exception as e:
        print(f"  Serial error: {e}")
    return ble_addr, lines


def trigger_warm_reboot(port_path):
    """Send warm reboot command over serial."""
    print(f"  Sending warm reboot via {port_path}...")
    try:
        with serial.Serial(port_path, 115200, timeout=1) as ser:
            ser.write(CMD_WARM_REBOOT)
            ser.flush()
        print("  Warm reboot sent!")
        return True
    except Exception as e:
        print(f"  Serial error: {e}")
        return False


def wait_for_port(timeout=15):
    """Wait for the USB serial port to reappear."""
    print("  Waiting for serial port to reappear...")
    for i in range(timeout):
        port = find_serial_port()
        if port:
            # Give it a moment to stabilize
            time.sleep(0.5)
            return port
        time.sleep(1)
    return None


def main():
    num_reboots = int(sys.argv[1]) if len(sys.argv) > 1 else 2

    # Find serial port
    port = find_serial_port()
    if not port:
        print("ERROR: No serial port found. Is the device connected?")
        sys.exit(1)
    print(f"Serial port: {port}\n")

    # Read current address by triggering a reboot and capturing boot output
    print("[1] Initial reboot to capture BLE_ADDR")
    if not trigger_warm_reboot(port):
        sys.exit(1)

    time.sleep(2)  # Wait for device to disconnect USB
    port = wait_for_port()
    if not port:
        print("FAIL: Device didn't come back after initial reboot")
        sys.exit(1)

    print("  Reading boot output...")
    addr_before, boot_lines = read_boot_output(port)
    if not addr_before:
        print("FAIL: Could not find BLE_ADDR in boot output")
        print("  Captured lines:")
        for l in boot_lines:
            print(f"    {l}")
        sys.exit(1)
    print(f"  BLE_ADDR: {addr_before}\n")

    # Reboot loop
    for i in range(num_reboots):
        print(f"[{i+2}] Warm reboot #{i+1}")
        if not trigger_warm_reboot(port):
            sys.exit(1)

        time.sleep(2)
        port = wait_for_port()
        if not port:
            print(f"FAIL: Device didn't come back after reboot #{i+1}")
            sys.exit(1)

        print("  Reading boot output...")
        addr_after, _ = read_boot_output(port)
        if not addr_after:
            print("FAIL: Could not find BLE_ADDR after reboot")
            sys.exit(1)

        match = addr_before == addr_after
        symbol = "✓ SAME" if match else "✗ DIFFERENT"
        print(f"  Before: {addr_before}")
        print(f"  After:  {addr_after}")
        print(f"  Result: {symbol}")
        print()

        if not match:
            print(f"FAIL: BLE address changed after reboot #{i+1}")
            sys.exit(1)

        addr_before = addr_after

    print(f"PASS: BLE address stable across {num_reboots} warm reboots!")


if __name__ == "__main__":
    main()
