#include <zephyr/kernel.h>
#include <zephyr/drivers/flash.h>
#include <stdint.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(cat_flash, LOG_LEVEL_INF);

#define SECTOR_SIZE 4096

#define FLASH_MAGIC 0xeebeefee

struct __attribute__((__packed__)) flash_header {
	uint32_t magic_number;
	uint32_t nrf70_fw_offset;
	uint32_t nrf70_fw_size;
	uint32_t persistance_offset;
} flash_header_read = {0};

bool did_post_flash = false;

const struct device *flash_dev = DEVICE_DT_GET_ONE(jedec_spi_nor);

void test_flash()
{
	flash_read(flash_dev, 0, &flash_header_read, sizeof(flash_header_read));
	LOG_INF("Flash header read. magic=%08x fw_offset=%08x fw_size=%08x persistance_offset=%08x",
		flash_header_read.magic_number, flash_header_read.nrf70_fw_offset,
		flash_header_read.nrf70_fw_size, flash_header_read.persistance_offset);

	if (flash_header_read.magic_number == FLASH_MAGIC) did_post_flash = true;

	// LOG_INF("Testing flash");

	// const struct device *flash_dev = DEVICE_DT_GET_ONE(jedec_spi_nor);

	// if (device_init(flash_dev))
	// {
	// 	printk("failed to init flash_dev");
	// }
	// else
	// {

	// 	if (!device_is_ready(flash_dev)) {
	// 		printk("%s: device not ready.\n", flash_dev->name);
	// 		// return 0;
	// 	}
	// 	else
	// 	{

	// 		printf("\n%s SPI flash testing\n", flash_dev->name);
	// 		printf("==========================\n");

	// 		single_sector_test(flash_dev);
	// 	}
	// }
}

int flash_get_nrf70_fw_size(int* size)
{
	if (flash_header_read.magic_number != FLASH_MAGIC) 
	{
		LOG_ERR("flash_get_nrf70_fw_size: magic_number bad (%08x)", flash_header_read.magic_number);
		return 1;
	}

	*size = flash_header_read.nrf70_fw_size;
	return 0;
}

int flash_load_nrf70_fw(uint8_t* target, uint8_t** fw_start, uint8_t** fw_end)
{
	if (flash_read(flash_dev, flash_header_read.nrf70_fw_offset, target, flash_header_read.nrf70_fw_size))
	{
		LOG_ERR("Failed to flash_read nrf70_fw_size");
		return 1;
	}

	*fw_start = target;
	*fw_end = target+flash_header_read.nrf70_fw_size;

	return 0;
}
