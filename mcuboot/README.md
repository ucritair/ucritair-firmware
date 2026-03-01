# CAT Bootloader (MCUboot)

Custom MCUboot bootloader for the cat5340 (nRF5340) board.

## Overview

- **MCUboot** single-slot configuration (no swap, no rollback)
- **USB DFU** mode for firmware flashing via `dfu-util`
- **GPREGRET** software-triggered DFU entry (app sets register, warm reboots)
- **Button combo** fallback: hold SELECT+START+DOWN while pressing RESET
- **No-application fallback**: enters DFU automatically if no valid app image
- **LCD display**: shows "CAT BOOTLOADER MODE" screen when in DFU
- **UICR init**: configures voltage regulator to 3.3V, manages power rails

## Partition Layout

| Partition | Address Range | Size | Label |
|-----------|--------------|------|-------|
| boot_partition | `0x00000` - `0x0E000` | 56 KB | catboot |
| slot0_partition | `0x0E000` - `0x100000` | 968 KB | image-0 |

Defined in: `zephyrapp/boards/nrf5340dk_nrf5340_cpuapp.overlay`

## DFU Entry Mechanisms

1. **GPREGRET (software-triggered)**: Application writes `0xB1` to GPREGRET register 0 and warm-reboots. Bootloader reads and clears the register. This is the BLE-triggered path.

2. **Button combo (manual)**: Hold SELECT+START+DOWN while pressing hardware RESET. Uses 4x2 button matrix scan (GPIO1 pins 9-14) with 50ms debounce.

3. **No valid application**: `CONFIG_BOOT_USB_DFU_NO_APPLICATION=y` enters DFU if the application image fails validation. This is a safety net.

## Custom Source Files

These files replace/extend the upstream MCUboot `boot/zephyr/` directory:

| File | Description |
|------|-------------|
| `io.c` | Full replacement for upstream `boot/zephyr/io.c`. Contains GPREGRET DFU check, 4x2 button matrix, LCD framebuffer (240x320 ST7789V), `indicate_waiting()` screen, and `board_cat_uicr_init()` SYS_INIT. |
| `font_8x8_basic.cpp` | 8x8 bitmap font data (public domain). `#include`d by `io.c`. |
| `mcuboot.patch` | Diff against upstream MCUboot `io.c` for reference. Generated against commit `3a195f22`. |
| `mcuboot.conf` | Kconfig overrides (single-slot, USB DFU, display, no signing). |
| `mcuboot.overlay` | Device tree overlay (selects `boot_partition` as code partition). |

## Building from Scratch

### Prerequisites

- Zephyr SDK with `west` installed
- MCUboot source (tested against commit `3a195f22`, v2.1.0-rc1)
- Zephyr patches applied (`zephyr.patch` in repo root — includes `usb_dfu.c` `indicate_waiting()` hook)

### Step 1: Apply custom code

Copy the custom files into your MCUboot tree:

```bash
cp mcuboot/io.c $ZEPHYR_BASE/../bootloader/mcuboot/boot/zephyr/io.c
cp mcuboot/font_8x8_basic.cpp $ZEPHYR_BASE/../bootloader/mcuboot/boot/zephyr/
```

Or apply the patch (plus copy the font file):

```bash
cd $ZEPHYR_BASE/../bootloader/mcuboot
git apply /path/to/cat_software/mcuboot/mcuboot.patch
cp /path/to/cat_software/mcuboot/font_8x8_basic.cpp boot/zephyr/
```

### Step 2: Build

```bash
west build -p always \
  -b cat5340/nrf5340/cpuapp \
  -d /path/to/cat_software/mcuboot/build \
  $ZEPHYR_BASE/../bootloader/mcuboot/boot/zephyr \
  -- \
  -DBOARD_ROOT=/path/to/cat_software/zephyrapp \
  -DEXTRA_CONF_FILE=/path/to/cat_software/mcuboot/mcuboot.conf \
  -DEXTRA_DTC_OVERLAY_FILE=/path/to/cat_software/mcuboot/mcuboot.overlay
```

Verify the output binary fits in the boot partition (56 KB):

```bash
ls -la mcuboot/build/zephyr/zephyr.bin  # must be < 57344 bytes
```

### Step 3: Flash (initial install requires J-Link)

```bash
west flash -d mcuboot/build
```

For subsequent updates, use the bootloader self-update mechanism (see `docs/firmware-update.md`).

## Signing Application Images

MCUboot requires image headers even with `CONFIG_BOOT_SIGNATURE_TYPE_NONE=y`. The signing step adds a SHA256 hash and metadata:

```bash
imgtool create --header-size 0x200 --align 8 \
  --version 0.1 --slot-size 0xF2000 --pad \
  build/zephyr/zephyr.bin build/zephyr/zephyr.signed.bin
```

The `--slot-size 0xF2000` (968 KB) must match `slot0_partition` in the DTS overlay.

## Regenerating the Patch

If you modify `io.c` in the MCUboot tree:

```bash
cd $ZEPHYR_BASE/../bootloader/mcuboot
git diff HEAD -- boot/zephyr/io.c > /path/to/cat_software/mcuboot/mcuboot.patch
cp boot/zephyr/io.c /path/to/cat_software/mcuboot/io.c
```

## USB DFU Device Identity

| Field | Value |
|-------|-------|
| VID | `0x2FE3` |
| PID (DFU mode) | `0x0100` |
| Device name | `MCUBOOT` |
