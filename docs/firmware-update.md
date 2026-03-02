# Firmware Update Guide

## Architecture Overview

```
+------------------+       USB serial        +------------------+
| Computer         | ---------------------> | Cat Device (App) |
| trigger_dfu.py   |  [CA 7D F0 01]         | lcd_rendering.c  |
| or flash.py      |  via CDC ACM           | usb_dfu_poll()   |
+------------------+                        +--------+---------+
                                                     |
                                               GPREGRET = 0xB1
                                               warm reboot
                                                     |
                                                     v
                                            +------------------+
                                            | MCUboot          |
                                            | reads GPREGRET   |
                                            | enters USB DFU   |
                                            +--------+---------+
                                                     |
                                               LCD: "CAT BOOTLOADER MODE"
                                               USB DFU device appears
                                                     |
                                                     v
+------------------+       USB DFU          +------------------+
| Computer         | ---------------------> | Cat Device (BL)  |
| dfu-util         |  zephyr.signed.bin     | MCUboot USB DFU  |
+------------------+                        +------------------+
                                                     |
                                               Verifies + boots new app
```

---

## New Device Setup

For brand new devices (or devices that need a bootloader upgrade), use the one-time init script:

```bash
cd cat_software
./scripts/device_init.sh
```

The device must be connected via USB-C and in DFU mode (hold SELECT+START+DOWN, press RESET).

The script will:
1. Check prerequisites (dfu-util, python3, pyserial, bleak)
2. Build firmware with embedded bootloader image
3. Flash via dfu-util and set the clock
4. Pause for you to run **Settings > DEVELOPER > UPDATE BOOTLOADER** on the device
5. Build and flash standard production firmware via `flash.py` (tests the new bootloader)
6. Run a BLE smoke test (GATT characteristics, ESS sensors, BTHome v2 advertising)

After this one-time setup, use Method 1 below for all future firmware updates.

Use `--skip-ble` to skip the BLE smoke test if `bleak` is not installed.

---

## Method 1: Automated Flash (Recommended)

Uses `flash.py` which handles the entire flow: trigger DFU via USB serial, wait for MCUboot, flash, and verify boot.

### Prerequisites

- Python 3.8+ with `pyserial`: `pip install pyserial`
- `dfu-util` installed (`brew install dfu-util` / `apt install dfu-util`)
- Device powered on and running application firmware
- USB cable connected between device and computer

### Steps

1. Build the firmware:

```bash
# In Docker:
docker compose run --rm dev
./utils/build.sh --embedded

# Or locally:
cd zephyrapp/game
./utils/build.sh --embedded --aq-first
```

2. Flash:

```bash
python3 scripts/flash.py zephyrapp/game/utils/cache/latest.bin
```

The script will:
- Find the USB serial port
- Send the DFU magic bytes to trigger MCUboot
- Wait for MCUboot USB DFU to enumerate
- Flash the firmware with dfu-util
- Verify the device boots successfully
- Set the device clock to the host's current time

---

## Method 2: Manual Steps

### Trigger DFU mode via USB serial

```bash
python3 scripts/trigger_dfu.py
```

The device reboots into MCUboot and the LCD shows "CAT BOOTLOADER MODE".

### Flash via USB

```bash
# Wait a few seconds for USB DFU to enumerate
dfu-util --alt 0 --download zephyrapp/build/zephyr/zephyr.signed.bin --reset
```

### One-liner

```bash
python3 scripts/trigger_dfu.py && sleep 3 && dfu-util --alt 0 --download zephyrapp/game/utils/cache/latest.bin --reset
```

---

## Method 3: Manual DFU (Button Combo Fallback)

Works without any scripts. No Python required.

1. Hold **SELECT + START + DOWN** buttons simultaneously
2. While holding the buttons, press **RESET**
3. LCD shows "CAT BOOTLOADER MODE"
4. Flash via USB:

```bash
dfu-util --alt 0 --download zephyrapp/build/zephyr/zephyr.signed.bin --reset
```

---

## Method 4: One-Time Bootloader Update

Devices with the original bootloader (no GPREGRET support) need a one-time bootloader update. After this, the USB serial DFU trigger works permanently.

### When is this needed?

- The device has never had its bootloader updated
- DFU trigger causes the device to just reboot back to the app (old bootloader ignores GPREGRET)

### Steps

1. Build firmware **with the embedded bootloader image**:

```bash
./utils/build.sh --embedded --bootloader
```

2. Flash this firmware via the **button combo** (Method 3). This is the last time the button combo is needed.

3. On the device, navigate to: **Settings > DEVELOPER > UPDATE BOOTLOADER**

4. The device will:
   - Check battery level (must be >= 20%)
   - Copy the embedded bootloader image to RAM
   - Erase the boot partition
   - Write the new bootloader
   - Read-back verify (up to 3 retry attempts)
   - Reboot with the new bootloader

5. After the update succeeds, all future builds can use the standard `--embedded` flag. The `--bootloader` flag is no longer needed.

---

## USB Serial Commands

The firmware polls the USB CDC ACM serial port for 4-byte magic sequences:

| Command | Magic Bytes | Effect |
|---------|-------------|--------|
| Enter DFU | `CA 7D F0 01` | Sets GPREGRET=0xB1, warm reboot into MCUboot DFU |
| Warm Reboot | `CA 7D BE 01` | Warm reboot (preserves RTC, no DFU) |
| Set Time | `CA 7D 54 04` + 4-byte LE uint32 | Sets RTC to Unix timestamp (seconds since epoch) |

These are used by `flash.py`, `trigger_dfu.py`, and `timing_test.py`.

---

## Regenerating the Embedded Bootloader Image

Only needed if you rebuild MCUboot with changes:

```bash
# Build MCUboot (see mcuboot/README.md)
# Then generate the .inc file:
xxd -i mcuboot/build/zephyr/zephyr.bin > zephyrapp/src/bootloader_image_data.inc

# Rebuild firmware with --bootloader to include it
./utils/build.sh --embedded --bootloader
```

---

## BLE Service Reference

**Service UUID**: `fc7d4395-1019-49c4-a91b-7491ecc40000`

Full characteristic UUID format: `fc7d4395-1019-49c4-a91b-7491ecc4XXXX`

| Char (XXXX) | Name | Access | Description |
|-------------|------|--------|-------------|
| `0x0001` | Device Name | R/W | Device identifier string |
| `0x0002` | Time | R/W | Unix timestamp (uint32) |
| `0x0003` | Cell Count | R | Log cell count |
| `0x0004` | Cell Selector | R/W | Select log cell to read |
| `0x0005` | Cell Data | R | Read selected log cell data |
| `0x0006` | Log Stream | W+N | Bulk log download via notifications (write `{start_cell: u32, count: u32}`, receive cells as notifications, end marker `0xFFFFFFFF`) |
| `0x0010` | Stats | R | Pet stats (vigour/focus/spirit/age) |
| `0x0011` | Items Owned | R | Owned items bitmap |
| `0x0012` | Items Placed | R | Placed items bitmap |
| `0x0013` | Bonus | R/W | Bonus value |
| `0x0014` | Pet Name | R/W | Pet name string |
| `0x0015` | Device Config | R/W | Device configuration (sensor period, sleep/dim timeouts, brightness, flags) |

### ESS Service (0x181A) — Environmental Sensing

Standard BLE Environmental Sensing characteristics with NOTIFY support:

| Characteristic | UUID | Access | Description |
|----------------|------|--------|-------------|
| Temperature | `0x2A6E` | R+N | sint16, 0.01°C resolution |
| Humidity | `0x2A6F` | R+N | uint16, 0.01% resolution |
| CO2 | `0x2B8C` | R+N | uint16, ppm |
| PM2.5 | `0x2BD6` | R+N | fp16 (IEEE 754 binary16), µg/m³ |
| Pressure | `0x2A6D` | R | uint32, 0.1 Pa resolution |
| PM1.0 | `0x2BD5` | R | fp16 (IEEE 754 binary16), µg/m³ |
| PM10 | `0x2BD7` | R | fp16 (IEEE 754 binary16), µg/m³ |

Subscribe to notifications (enable CCC descriptor) for real-time sensor updates (~5s interval while device is awake).

### BTHome v2 Advertising (Home Assistant)

The device broadcasts [BTHome v2](https://bthome.io/) sensor data in its advertising packets for passive integration with Home Assistant — no pairing required.

**Service UUID**: `0xFCD2`

**Advertised sensors**:

| Sensor | BTHome Object ID | Format |
|--------|-----------------|--------|
| Battery | `0x01` | uint8, % |
| Temperature | `0x02` | sint16, factor 0.01°C |
| Humidity | `0x03` | uint16, factor 0.01% |
| PM2.5 | `0x0D` | uint16, µg/m³ |
| PM10 | `0x0E` | uint16, µg/m³ |
| CO2 | `0x12` | uint16, ppm |

**Behavior**:
- While awake: connectable advertising with BTHome data, refreshed every ~5s
- During timer-wake (sensor logging): 3-second non-connectable BTHome broadcast after each sensor reading
- While sleeping: no advertising (device is in deep sleep between sensor cycles)

Home Assistant auto-discovers the device via BTHome integration.

### Key Constants

| Constant | Value | Used By |
|----------|-------|---------|
| GPREGRET magic | `0xB1` | App (`bl_update.h`) and bootloader (`io.c`) |
| USB VID | `0x2FE3` | MCUboot DFU mode |
| USB PID (DFU) | `0x0100` | MCUboot DFU mode |

---

## Troubleshooting

### `flash.py` / `trigger_dfu.py`: No serial port found

- Ensure the device is powered on and connected via USB
- Check that a serial port appears: `ls /dev/tty.usbmodem*` (macOS) or `ls /dev/ttyACM*` (Linux)
- Specify the port manually: `--port /dev/ttyACM0`

### `dfu-util`: No DFU capable USB device available

- Wait 3-5 seconds after triggering DFU for USB enumeration
- On **Windows**: run Zadig to install WinUSB driver for the `MCUBOOT` device
- On **Linux**: check udev rules (see main README.md)
- Verify with `dfu-util -l` that the device appears (VID:PID `2FE3:0100`)

### DFU trigger causes device to just reboot normally

- The bootloader does not have GPREGRET support (old bootloader)
- Follow "Method 4: One-Time Bootloader Update" to install the new bootloader

### "UPDATE BOOTLOADER" not shown in developer menu

- The firmware was not built with the `--bootloader` flag
- Rebuild with: `./utils/build.sh --embedded --bootloader`

### Bootloader update failed

- Check battery level is above 20%
- Ensure USB power is connected for stable supply
- The update retries up to 3 times automatically
- Worst case: use J-Link/nrfjprog to reflash the bootloader via SWD

### Device stuck in bootloader mode

- Press RESET without holding any buttons to attempt normal boot
- If no valid app image exists, the bootloader stays in DFU mode by design
- Flash a valid signed image via `dfu-util` to restore normal operation
