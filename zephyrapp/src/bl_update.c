#include "bl_update.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/sys/reboot.h>
#include <hal/nrf_power.h>
#include <zephyr/logging/log.h>
#include <string.h>

LOG_MODULE_REGISTER(bl_update, LOG_LEVEL_INF);

#include "batt.h"

void bl_enter_dfu(void)
{
	nrf_power_gpregret_set(NRF_POWER, 0, DFU_MAGIC_VALUE);
	sys_reboot(SYS_REBOOT_WARM);
}

#ifdef BL_IMAGE_INCLUDED

static const unsigned char bootloader_image[] = {
#include "bootloader_image_data.inc"
};
static const unsigned int bootloader_image_len = sizeof(bootloader_image);

bool bl_image_included(void)
{
	return true;
}

int bl_update_flash(void)
{
	LOG_INF("=== BOOTLOADER SELF-UPDATE STARTING ===");

	/* 1. Battery check */
	int batt = get_battery_pct();
	if (batt < 20)
	{
		LOG_ERR("Battery too low (%d%%), need >= 20%%", batt);
		return -1;
	}
	LOG_INF("Battery: %d%% (OK)", batt);

	/* 2. Copy bootloader image from flash .rodata into RAM.
	 *    This is critical: the NVMC halts CPU flash access during
	 *    erase/write. We must not read source data from flash while
	 *    writing to flash.
	 *    Static buffer avoids heap dependency (system heap is only 4KB). */
	static uint8_t ram_copy[(sizeof(bootloader_image) + 3u) & ~3u] __aligned(4);
	memcpy(ram_copy, bootloader_image, bootloader_image_len);

	/* 3. Verify the RAM copy matches the flash original (catch RAM corruption) */
	if (memcmp(ram_copy, bootloader_image, bootloader_image_len) != 0)
	{
		LOG_ERR("RAM copy verification failed!");
		return -3;
	}

	/* 4. Verify ARM vector table sanity in the image */
	if (bootloader_image_len < 16)
	{
		LOG_ERR("Image too small (%u bytes)", bootloader_image_len);
		return -4;
	}
	uint32_t sp, reset_vec;
	memcpy(&sp, ram_copy, 4);
	memcpy(&reset_vec, ram_copy + 4, 4);
	/* SP should be in SRAM (0x20000000 range) */
	if (sp < 0x20000000 || sp >= 0x20100000)
	{
		LOG_ERR("Invalid SP in vector table: 0x%08x", sp);
		return -4;
	}
	/* Reset vector should be in boot partition flash, Thumb bit set */
	if (reset_vec < 1 || reset_vec >= 0x0E000 || !(reset_vec & 1))
	{
		LOG_ERR("Invalid reset vector: 0x%08x", reset_vec);
		return -4;
	}
	LOG_INF("Vector table: SP=0x%08x Reset=0x%08x (OK)", sp, reset_vec);

	/* 5. Open boot partition */
	const struct flash_area *fa;
	int rc = flash_area_open(FIXED_PARTITION_ID(boot_partition), &fa);
	if (rc)
	{
		LOG_ERR("Failed to open boot partition: %d", rc);
		return -5;
	}

	if (bootloader_image_len > fa->fa_size)
	{
		LOG_ERR("Image (%u) exceeds partition (%u)",
			bootloader_image_len, fa->fa_size);
		flash_area_close(fa);
		return -5;
	}
	LOG_INF("Boot partition: offset=0x%x size=0x%x", (unsigned)fa->fa_off, (unsigned)fa->fa_size);

	/* Pad write length to 4-byte boundary (nRF5340 flash requires word-aligned writes).
	 * The static ram_copy buffer is __aligned(4) and sizeof(bootloader_image) rounded up
	 * fits within it. Padding bytes are irrelevant since the region was erased to 0xFF. */
	unsigned int write_len = (bootloader_image_len + 3u) & ~3u;
	LOG_INF("Image size: %u, write size (aligned): %u", bootloader_image_len, write_len);

	/* === POINT OF NO RETURN === */
	/* Retry loop: erase → write → verify. Up to 3 attempts. */
	#define BL_UPDATE_MAX_ATTEMPTS 3
	bool update_ok = false;

	for (int attempt = 1; attempt <= BL_UPDATE_MAX_ATTEMPTS; attempt++)
	{
		LOG_INF("--- Attempt %d/%d ---", attempt, BL_UPDATE_MAX_ATTEMPTS);

		/* Erase */
		LOG_INF("Erasing boot partition...");
		rc = flash_area_erase(fa, 0, fa->fa_size);
		if (rc)
		{
			LOG_ERR("CRITICAL: Erase failed: %d", rc);
			continue;
		}

		/* Write */
		LOG_INF("Writing bootloader (%u bytes from RAM)...", write_len);
		rc = flash_area_write(fa, 0, ram_copy, write_len);
		if (rc)
		{
			LOG_ERR("CRITICAL: Write failed: %d", rc);
			continue;
		}

		/* Read-back verify */
		LOG_INF("Verifying...");
		uint8_t readback[256];
		bool verify_ok = true;
		for (unsigned int offset = 0; offset < bootloader_image_len; offset += sizeof(readback))
		{
			unsigned int chunk = bootloader_image_len - offset;
			if (chunk > sizeof(readback))
				chunk = sizeof(readback);

			rc = flash_area_read(fa, offset, readback, chunk);
			if (rc)
			{
				LOG_ERR("Read-back failed at offset %u: %d", offset, rc);
				verify_ok = false;
				break;
			}

			if (memcmp(readback, &ram_copy[offset], chunk) != 0)
			{
				LOG_ERR("Verify mismatch at offset %u", offset);
				verify_ok = false;
				break;
			}
		}

		if (verify_ok)
		{
			update_ok = true;
			break;
		}

		LOG_ERR("Verify failed on attempt %d", attempt);
	}

	flash_area_close(fa);

	if (!update_ok)
	{
		LOG_ERR("CRITICAL: All %d attempts failed — DEVICE MAY NOT BOOT", BL_UPDATE_MAX_ATTEMPTS);
		return -8;
	}

	LOG_INF("=== BOOTLOADER UPDATE VERIFIED OK ===");
	LOG_INF("Rebooting in 1 second...");
	k_msleep(1000);
	sys_reboot(SYS_REBOOT_COLD);

	return 0; /* unreachable */
}

#else /* BL_IMAGE_INCLUDED not defined */

bool bl_image_included(void)
{
	return false;
}

int bl_update_flash(void)
{
	LOG_ERR("Bootloader image not included in this build");
	return -1;
}

#endif /* BL_IMAGE_INCLUDED */
