# Firmware Update Guide

## Architecture Overview

```
+------------------+       BLE write        +------------------+
| Computer/Phone   | ---------------------> | Cat Device (App) |
| trigger_dfu.py   |  char 0x0009           | ble.c            |
+------------------+  [CA 7D F0 01]         +--------+---------+
                                                     |
                                               bl_enter_dfu()
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

## Method 1: BLE-Triggered DFU (No Buttons Required)

This is the primary update method. Requires the updated bootloader with GPREGRET support.

### Prerequisites

- Python 3.8+ with `bleak`: `pip install bleak`
- `dfu-util` installed (see main README for platform-specific setup)
- Device powered on and running application firmware
- Bluetooth enabled on computer
- USB cable connected between device and computer

### Steps

1. Build the firmware:

```bash
# In Docker:
docker compose run --rm dev
./utils/build.sh --embedded

# Or locally with west:
west build -p always -b cat5340/nrf5340/cpuapp ...
```

2. Trigger DFU mode over BLE:

```bash
python3 scripts/trigger_dfu.py
```

The device LCD will show "CAT BOOTLOADER MODE". The script will report "disconnected" which is expected (the device rebooted).

3. Flash via USB:

```bash
dfu-util --download zephyrapp/build/zephyr/zephyr.signed.bin --reset
```

The device reboots into the new firmware automatically.

### One-liner (trigger + wait + flash)

```bash
python3 scripts/trigger_dfu.py && sleep 3 && dfu-util --download zephyrapp/build/zephyr/zephyr.signed.bin --reset
```

---

## Method 2: Manual DFU (Button Combo Fallback)

Works with any bootloader version. No BLE or Python required.

1. Hold **SELECT + START + DOWN** buttons simultaneously
2. While holding the buttons, press **RESET** (may need a paperclip)
3. LCD shows "CAT BOOTLOADER MODE"
4. Flash via USB:

```bash
dfu-util --download zephyrapp/build/zephyr/zephyr.signed.bin --reset
```

---

## Method 3: One-Time Bootloader Update

Devices with the original bootloader (no GPREGRET support) need a one-time bootloader update. After this update, BLE-triggered DFU (Method 1) works permanently.

### When is this needed?

- The device has never had its bootloader updated
- BLE DFU trigger causes the device to just reboot back to the app (old bootloader ignores GPREGRET)

### Steps

1. Build firmware **with the embedded bootloader image**:

```bash
./utils/build.sh --embedded --bootloader
```

The `--bootloader` flag includes the ~53 KB bootloader binary in the firmware image. This is only needed for this one-time update.

2. Flash this firmware via the **old method** (button combo, Method 2). This is the last time the button combo is needed.

3. On the device, navigate to: **Settings > DEVELOPER > UPDATE BOOTLOADER**

   Or trigger remotely via BLE:

```bash
python3 scripts/trigger_dfu.py --bl-update
```

4. The device will:
   - Check battery level (must be >= 20%)
   - Copy the embedded bootloader image to RAM
   - Erase the boot partition
   - Write the new bootloader from RAM
   - Read-back verify (up to 3 retry attempts)
   - Reboot with the new bootloader

5. After the update succeeds, all future firmware builds can use the normal `--embedded` flag without `--bootloader`. The embedded bootloader image is no longer needed.

### Subsequent firmware updates

After the one-time bootloader update, the standard workflow is:

```bash
./utils/build.sh --embedded          # no --bootloader needed
python3 scripts/trigger_dfu.py       # BLE trigger
sleep 3
dfu-util --download zephyrapp/build/zephyr/zephyr.signed.bin --reset
```

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

| Char (XXXX) | Name | Access | Magic Bytes | Description |
|-------------|------|--------|-------------|-------------|
| `0x0001` | Device Name | R/W | - | Device identifier string |
| `0x0002` | Time | R/W | - | Unix timestamp (uint32) |
| `0x0003` | Cell Count | R | - | Log cell count |
| `0x0004` | Cell Selector | R/W | - | Select log cell to read |
| `0x0005` | Cell Data | R | - | Read selected log cell data |
| `0x0009` | DFU Control | W | `CA 7D F0 01` | Triggers DFU mode (GPREGRET reboot) |
| `0x000A` | BL Update | W | `CA 7D B0 07` | Triggers bootloader self-update |
| `0x0010` | Stats | R | - | Pet stats (vigour/focus/spirit/age) |
| `0x0013` | Bonus | R/W | - | Bonus value |
| `0x0014` | Pet Name | R/W | - | Pet name string |

### Key Constants

| Constant | Value | Used By |
|----------|-------|---------|
| GPREGRET magic | `0xB1` | App (`bl_update.h`) and bootloader (`io.c`) |
| USB VID | `0x2FE3` | MCUboot DFU mode |
| USB PID (DFU) | `0x0100` | MCUboot DFU mode |

---

## Troubleshooting

### `trigger_dfu.py`: Device not found

- Ensure device is powered on and running app firmware (not already in bootloader mode)
- Check Bluetooth is enabled on your computer
- The script scans by service UUID first, then falls back to scanning by name ("ucritair")
- Try increasing the timeout: `python3 scripts/trigger_dfu.py --timeout 30`

### `dfu-util`: No DFU capable USB device available

- Ensure a USB cable is connected (BLE trigger only puts the device in DFU mode; actual flashing is over USB)
- Wait 3-5 seconds after triggering DFU for USB enumeration
- On **Windows**: run Zadig to install WinUSB driver for the `MCUBOOT` device
- On **Linux**: check udev rules (see main README.md)
- Verify with `dfu-util -l` that the device appears (VID:PID `2FE3:0100`)

### BLE DFU trigger causes device to just reboot normally

- The bootloader does not have GPREGRET support (old bootloader)
- Follow "Method 3: One-Time Bootloader Update" to install the new bootloader

### "UPDATE BOOTLOADER" not shown in developer menu

- The firmware was not built with the `--bootloader` flag
- Rebuild with: `./utils/build.sh --embedded --bootloader`

### Bootloader update failed

- Check battery level is above 20%
- Ensure USB power is connected for stable supply
- The update retries up to 3 times automatically
- If all attempts fail, the device may still boot if the old bootloader was partially preserved
- Worst case: use J-Link/nrfjprog to reflash the bootloader via SWD

### Device stuck in bootloader mode

- Press RESET without holding any buttons to attempt normal boot
- If no valid app image exists, the bootloader stays in DFU mode by design
- Flash a valid signed image via `dfu-util` to restore normal operation
