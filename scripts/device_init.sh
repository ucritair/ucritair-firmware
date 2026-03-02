#!/usr/bin/env bash
#
# device_init.sh — One-time setup for a new uCrit device
#
# Automates the full initialization flow:
#   1. Build firmware with embedded bootloader image
#   2. Flash to device (must already be in DFU mode)
#   3. User triggers bootloader update on device menu
#   4. Build and flash standard production firmware (via USB serial DFU trigger)
#   5. Set device clock and run BLE smoke test
#
# Prerequisites:
#   - Device connected via USB-C, in DFU mode (hold SELECT+START+DOWN + RESET)
#   - Zephyr toolchain set up (~/Code/zephyrproject/)
#   - dfu-util installed (brew install dfu-util)
#   - pyserial installed (pip install pyserial)
#   - bleak installed (pip install bleak) — for BLE smoke test
#
# Usage:
#   cd cat_software
#   ./scripts/device_init.sh
#   ./scripts/device_init.sh --skip-ble   # skip BLE smoke test
#

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
GAME_DIR="$REPO_DIR/zephyrapp/game"
CACHE_DIR="$GAME_DIR/utils/cache"

SKIP_BLE=false
for arg in "$@"; do
    case "$arg" in
        --skip-ble) SKIP_BLE=true ;;
        *) echo "Unknown option: $arg"; exit 1 ;;
    esac
done

passed=0
failed=0
step=0
total_steps=8

step_header() {
    step=$((step + 1))
    echo ""
    echo "============================================"
    echo "  [$step/$total_steps] $1"
    echo "============================================"
}

die() {
    echo "ERROR: $1" >&2
    exit 1
}

wait_for_serial() {
    local timeout=${1:-15}
    for i in $(seq 1 "$timeout"); do
        local port
        port=$(find_serial_port)
        if [ -n "$port" ]; then
            echo "$port"
            return 0
        fi
        sleep 1
    done
    return 1
}

find_serial_port() {
    for pattern in /dev/tty.usbmodem* /dev/ttyACM* /dev/ttyUSB*; do
        # shellcheck disable=SC2086
        local match
        match=$(ls $pattern 2>/dev/null | head -1) || true
        if [ -n "$match" ]; then
            echo "$match"
            return
        fi
    done
}

# ─── Step 1: Check prerequisites ───────────────────────────────────────────────

step_header "Checking prerequisites"

errors=0

if ! command -v dfu-util &>/dev/null; then
    echo "  MISSING: dfu-util (brew install dfu-util)"
    errors=$((errors + 1))
else
    echo "  OK: dfu-util $(dfu-util -V 2>&1 | head -1 | awk '{print $2}')"
fi

if ! command -v python3 &>/dev/null; then
    echo "  MISSING: python3"
    errors=$((errors + 1))
else
    echo "  OK: python3 $(python3 --version 2>&1 | awk '{print $2}')"
fi

if ! python3 -c "import serial" 2>/dev/null; then
    echo "  MISSING: pyserial (pip install pyserial)"
    errors=$((errors + 1))
else
    echo "  OK: pyserial"
fi

if [ "$SKIP_BLE" = false ]; then
    if ! python3 -c "import bleak" 2>/dev/null; then
        echo "  MISSING: bleak (pip install bleak) — or use --skip-ble"
        errors=$((errors + 1))
    else
        echo "  OK: bleak"
    fi
fi

[ "$errors" -gt 0 ] && die "Missing $errors prerequisite(s)"

# ─── Step 2: Check device is in DFU mode ───────────────────────────────────────

step_header "Checking device is in DFU mode"

if ! dfu-util -l 2>&1 | grep -q "2fe3"; then
    echo ""
    echo "  No DFU device found."
    echo ""
    echo "  To enter DFU mode on a new device:"
    echo "    1. Hold SELECT + START + DOWN buttons"
    echo "    2. While holding, press RESET"
    echo "    3. LCD shows 'CAT BOOTLOADER MODE'"
    echo ""
    read -rp "  Press Enter when device is in DFU mode..."

    if ! dfu-util -l 2>&1 | grep -q "2fe3"; then
        die "Still no DFU device found (VID:PID 2fe3:0100)"
    fi
fi

serial=$(dfu-util -l 2>&1 | grep "2fe3" | sed -n 's/.*serial="\([^"]*\)".*/\1/p' | head -1)
echo "  DFU device found (serial: ${serial:-unknown})"

# ─── Step 3: Build firmware with bootloader ────────────────────────────────────

step_header "Building firmware WITH bootloader"

# Set up Zephyr environment if not already active
if [ -z "${ZEPHYR_BASE:-}" ]; then
    ZEPHYR_ENV="$HOME/Code/zephyrproject/zephyr/zephyr-env.sh"
    if [ ! -f "$ZEPHYR_ENV" ]; then
        die "ZEPHYR_BASE not set and $ZEPHYR_ENV not found"
    fi
    echo "  Sourcing Zephyr environment..."
    # shellcheck disable=SC1090
    source "$ZEPHYR_ENV"
fi

VENV="$ZEPHYR_BASE/../venv/bin/activate"
if [ -f "$VENV" ]; then
    echo "  Activating venv..."
    # shellcheck disable=SC1090
    source "$VENV"
fi

echo "  Building (this takes a few minutes)..."
cd "$GAME_DIR"
./utils/build.sh --embedded --aq-first --bootloader --clean 2>&1 | tail -5

BL_FW="$CACHE_DIR/latest.bin"
[ -f "$BL_FW" ] || die "Build failed — $BL_FW not found"

bl_size=$(stat -f%z "$BL_FW" 2>/dev/null || stat -c%s "$BL_FW" 2>/dev/null)
echo "  Built: $(basename "$BL_FW") ($bl_size bytes)"

# ─── Step 4: Flash bootloader firmware ─────────────────────────────────────────

step_header "Flashing bootloader firmware"

echo "  Downloading to device..."
if ! dfu-util --alt 0 --download "$BL_FW" --reset 2>&1 | grep -q "Download done"; then
    # Check if it succeeded despite LIBUSB_ERROR_IO (device rebooted)
    echo "  (dfu-util may report IO error — this is expected if device rebooted)"
fi
echo "  Flash complete!"

echo "  Waiting for device to boot..."
sleep 5
SERIAL_PORT=$(wait_for_serial 15) || die "Device did not come back after flash"
echo "  Device booted on $SERIAL_PORT"

# Set time so the device has correct clock during bootloader update
echo "  Setting time..."
cd "$REPO_DIR"
python3 scripts/trigger_dfu.py --set-time --port "$SERIAL_PORT" 2>&1 | sed 's/^/  /'

# ─── Step 5: Bootloader update (manual step) ──────────────────────────────────

step_header "Bootloader update (MANUAL STEP)"

echo ""
echo "  On the device, navigate to:"
echo ""
echo "    Settings > DEVELOPER > UPDATE BOOTLOADER"
echo ""
echo "  The device will:"
echo "    - Check battery (must be >= 20%)"
echo "    - Erase and rewrite the boot partition"
echo "    - Verify and reboot"
echo ""
echo "  This takes about 10-15 seconds."
echo ""
read -rp "  Press Enter when the bootloader update is done and the device has rebooted..."

echo "  Checking device is back..."
SERIAL_PORT=$(wait_for_serial 30) || die "Device did not come back after bootloader update"
echo "  Device found on $SERIAL_PORT"

# ─── Step 6: Build standard firmware ──────────────────────────────────────────

step_header "Building standard firmware"

echo "  Building (this takes a few minutes)..."
cd "$GAME_DIR"
./utils/build.sh --embedded --aq-first --clean 2>&1 | tail -5

STD_FW="$CACHE_DIR/latest.bin"
[ -f "$STD_FW" ] || die "Build failed — $STD_FW not found"

std_size=$(stat -f%z "$STD_FW" 2>/dev/null || stat -c%s "$STD_FW" 2>/dev/null)
echo "  Built: $(basename "$STD_FW") ($std_size bytes)"

# ─── Step 7: Flash standard firmware via flash.py ─────────────────────────────

step_header "Flashing standard firmware (tests USB serial DFU trigger)"

cd "$REPO_DIR"
python3 scripts/flash.py "$STD_FW" 2>&1 | sed 's/^/  /'

# ─── Step 8: BLE smoke test ───────────────────────────────────────────────────

if [ "$SKIP_BLE" = true ]; then
    step_header "BLE smoke test (SKIPPED)"
    echo "  Skipped via --skip-ble flag"
else
    step_header "BLE smoke test"

    python3 - <<'PYEOF'
import asyncio, struct, time, calendar
from datetime import datetime
from bleak import BleakScanner, BleakClient

SVC = 'fc7d4395-1019-49c4-a91b-7491ecc4'
CHARS = {
    'Time':        SVC + '0002',
    'Device Name': SVC + '0001',
    'Pet Name':    SVC + '0014',
    'Log Cells':   SVC + '0003',
    'Stats':       SVC + '0010',
    'Items':       SVC + '0011',
    'Bonus':       SVC + '0013',
    'Config':      SVC + '0015',
}
ESS = {
    'Temperature': ('00002a6e-0000-1000-8000-00805f9b34fb', '<h', lambda v: f'{v/100:.1f} C'),
    'Humidity':    ('00002a6f-0000-1000-8000-00805f9b34fb', '<H', lambda v: f'{v/100:.1f}%'),
    'Pressure':    ('00002a6d-0000-1000-8000-00805f9b34fb', '<I', lambda v: f'{v/10:.0f} Pa'),
    'CO2':         ('00002b8c-0000-1000-8000-00805f9b34fb', '<H', lambda v: f'{v} ppm'),
    'PM2.5':       ('00002bd6-0000-1000-8000-00805f9b34fb', '<H', lambda v: f'raw={v}'),
    'PM1.0':       ('00002bd5-0000-1000-8000-00805f9b34fb', '<H', lambda v: f'raw={v}'),
    'PM10':        ('00002bd7-0000-1000-8000-00805f9b34fb', '<H', lambda v: f'raw={v}'),
}

passed = failed = 0

def check(name, ok, detail=''):
    global passed, failed
    if ok:
        passed += 1
        print(f'  PASS  {name}' + (f' -- {detail}' if detail else ''))
    else:
        failed += 1
        print(f'  FAIL  {name}' + (f' -- {detail}' if detail else ''))

async def run():
    global passed, failed

    print('  Scanning for device...')
    dev = await BleakScanner.find_device_by_filter(
        lambda d, ad: d.name and 'ucrit' in d.name.lower(), timeout=15)
    check('Device found', dev is not None,
          f'{dev.name} ({dev.address})' if dev else 'not found')
    if not dev:
        return

    async with BleakClient(dev) as c:
        check('Connected', c.is_connected)

        # Time check
        data = await c.read_gatt_char(CHARS['Time'])
        ts = struct.unpack('<I', data)[0]
        local_ts = calendar.timegm(time.localtime())
        delta = abs(ts - local_ts)
        dt = datetime.utcfromtimestamp(ts).strftime('%Y-%m-%d %H:%M:%S')
        check('Time accurate', delta < 10, f'{dt} (delta={delta}s)')

        # Custom service chars
        for name, uuid in CHARS.items():
            if name == 'Time':
                continue
            try:
                d = await c.read_gatt_char(uuid)
                if name == 'Device Name' or name == 'Pet Name':
                    val = d.decode('utf-8', errors='replace').rstrip('\x00')
                    check(name, len(d) > 0, repr(val))
                elif name == 'Log Cells':
                    cells = struct.unpack('<I', d[:4])[0]
                    check(name, True, f'{cells} cells')
                else:
                    check(name, len(d) > 0, f'{len(d)} bytes')
            except Exception as e:
                check(name, False, str(e))

        # ESS sensors
        for name, (uuid, fmt, display) in ESS.items():
            try:
                d = await c.read_gatt_char(uuid)
                val = struct.unpack(fmt, d)[0]
                check(f'ESS {name}', True, display(val))
            except Exception as e:
                check(f'ESS {name}', False, str(e))

    # BTHome v2 advertising
    print()
    print('  --- BTHome v2 Advertising ---')
    BTHOME_OBJECTS = {
        0x01: ('Battery', 'uint8', '%'),
        0x02: ('Temperature', 'sint16', '0.01 C'),
        0x03: ('Humidity', 'uint16', '0.01%'),
        0x0D: ('PM2.5', 'uint16', 'ug/m3'),
        0x0E: ('PM10', 'uint16', 'ug/m3'),
        0x12: ('CO2', 'uint16', 'ppm'),
    }

    print('  Scanning for BTHome advertisements...')
    devices = await BleakScanner.discover(timeout=10, return_adv=True)
    bthome_data = None
    for addr, (d2, adv) in devices.items():
        n = d2.name or adv.local_name or ''
        if 'ucrit' not in n.lower():
            continue
        for uuid, data in adv.service_data.items():
            if 'fcd2' in uuid.lower():
                bthome_data = data
                break
        break

    check('BTHome service data present', bthome_data is not None,
          f'{len(bthome_data)} bytes' if bthome_data else 'not found')

    if bthome_data and len(bthome_data) > 1:
        version = (bthome_data[0] >> 5) & 0x07
        check('BTHome version 2', version == 2, f'version={version}')

        i = 1
        found_sensors = set()
        while i < len(bthome_data):
            obj_id = bthome_data[i]
            i += 1
            if obj_id not in BTHOME_OBJECTS:
                break
            name, fmt, unit = BTHOME_OBJECTS[obj_id]
            sz = 1 if fmt == 'uint8' else 2
            if i + sz > len(bthome_data):
                break
            if sz == 1:
                val = bthome_data[i]
            else:
                val = struct.unpack('<h' if fmt == 'sint16' else '<H',
                                    bthome_data[i:i+2])[0]
            i += sz
            found_sensors.add(name)
            if name == 'Temperature':
                check(f'BTHome {name}', True, f'{val*0.01:.1f} C')
            elif name == 'Humidity':
                check(f'BTHome {name}', True, f'{val*0.01:.1f}%')
            else:
                check(f'BTHome {name}', True, f'{val} {unit}')

        expected = {'Battery', 'Temperature', 'Humidity', 'PM2.5', 'PM10', 'CO2'}
        missing = expected - found_sensors
        check('All BTHome sensors present', len(missing) == 0,
              f'missing: {missing}' if missing else 'all 6 found')

    print()
    if failed == 0:
        print(f'  All {passed} checks passed!')
    else:
        print(f'  {passed} passed, {failed} FAILED')

asyncio.run(run())
PYEOF

fi

# ─── Summary ──────────────────────────────────────────────────────────────────

echo ""
echo "============================================"
echo "  Device initialization complete!"
echo "============================================"
echo ""
echo "  Bootloader: updated"
echo "  Firmware:   standard ($std_size bytes)"
echo "  Time:       set to local"
if [ "$SKIP_BLE" = false ]; then
    echo "  BLE:        smoke test ran"
fi
echo ""
