#include <zephyr/kernel.h>
#include <zephyr/drivers/flash.h>
#include <stdint.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(cat_flash, LOG_LEVEL_DBG);

#include "airquality.h"
#include "rtc.h"

#define SECTOR_SIZE 4096

#define FLASH_MAGIC 0xeebeefee

struct __attribute__((__packed__)) flash_header {
	uint32_t magic_number;
	uint32_t nrf70_fw_offset;
	uint32_t nrf70_fw_size;
	uint32_t persistance_offset;
} flash_header_read = {0};

#define ROOM_FOR_TOMAS 0x10000

struct __attribute__((__packed__)) flash_log_cell {
	uint8_t flags;
	uint8_t unused[3];

	uint32_t timestamp_s; // epoch time timestamp in seconds
	int32_t temp_Cx1000; // C * 1000

	uint16_t rh_pctx100; // % * 100
	uint16_t co2_ppmx1; // ppm * 1
	uint16_t pm_ugmx100[4]; //1.0, 2.5, 4.0, 10.0 x100

	uint8_t voc_index, nox_index; //x1

	uint8_t pad[6];
};

static inline bool cell_is_valid(struct flash_log_cell* cell)
{
	return cell->timestamp_s != 0 && cell->timestamp_s != UINT32_MAX;
}


#define PERSISTANCE_OFFSET (flash_header_read.persistance_offset + ROOM_FOR_TOMAS)
#define MAX_LOG_CELL_COUNT (((0x1000000 - PERSISTANCE_OFFSET) / sizeof(struct flash_log_cell)) & ~1)

#define LOG_CELL_NR_FULL -1
#define LOG_CELL_NR_UNKOWN -2

PERSIST_RAM int next_log_cell_nr = LOG_CELL_NR_UNKOWN;
int flash_get_next_log_cell_nr();

_Static_assert(sizeof(struct flash_log_cell) == 32);

bool did_post_flash = false;

const struct device *flash_dev = DEVICE_DT_GET_ONE(jedec_spi_nor);

void test_flash()
{
	flash_read(flash_dev, 0, &flash_header_read, sizeof(flash_header_read));
	LOG_INF("Flash header read. magic=%08x fw_offset=%08x fw_size=%08x persistance_offset=%08x",
		flash_header_read.magic_number, flash_header_read.nrf70_fw_offset,
		flash_header_read.nrf70_fw_size, flash_header_read.persistance_offset);

	if (flash_header_read.magic_number == FLASH_MAGIC) did_post_flash = true;

	next_log_cell_nr = flash_get_next_log_cell_nr();
	LOG_INF("next_log_cell_nr = %d", next_log_cell_nr);
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

#define OFFSET_OF_CELL(nr) (PERSISTANCE_OFFSET + (sizeof(struct flash_log_cell) * nr))

void flash_get_cell_by_nr(int nr, struct flash_log_cell* out)
{
	LOG_DBG("%s(%d) (@%x)", __func__, nr, OFFSET_OF_CELL(nr));
	flash_read(flash_dev, OFFSET_OF_CELL(nr), out, sizeof(struct flash_log_cell));
}

void flash_write_cell_by_nr(int nr, struct flash_log_cell* out)
{
	LOG_DBG("%s(%d) (@%x)", __func__, nr, OFFSET_OF_CELL(nr));
	flash_write(flash_dev, OFFSET_OF_CELL(nr), out, sizeof(struct flash_log_cell));
}

int flash_get_next_log_cell_nr()
{
	struct flash_log_cell cell;

	LOG_DBG("flash_get_next_log_cell_nr: MAX_LOG_CELL_COUNT=%d", MAX_LOG_CELL_COUNT);

	int check = 0;
	int stepsize = 0x1000;

	while (check < MAX_LOG_CELL_COUNT)
	{
		flash_get_cell_by_nr(check, &cell);
		LOG_DBG("cell_is_valid[%d]->%d", check, cell_is_valid(&cell));

		if (cell_is_valid(&cell))
		{
			check += stepsize;
		}
		else
		{
			if (check == 0)
				return 0;

			if (stepsize == 1)
			{
				return check;
			}
			else
			{
				check -= stepsize;
				stepsize >>= 1;
				check += stepsize;
			}
		}
	}

	return LOG_CELL_NR_FULL;
}

void populate_log_cell(struct flash_log_cell* cell)
{
	cell->timestamp_s = get_current_rtc_time(); // epoch time timestamp in seconds

	cell->temp_Cx1000 = current_readings.sen5x.temp_degC * 1000; // C * 1000
	cell->rh_pctx100 = current_readings.sen5x.humidity_rhpct * 100; // % * 100

	cell->co2_ppmx1 = current_readings.sunrise.ppm_filtered_compensated * 1; // ppm * 1

	cell->pm_ugmx100[0] = current_readings.sen5x.pm1_0 * 100; //1.0, 2.5, 4.0, 10.0 x100
	cell->pm_ugmx100[1] = current_readings.sen5x.pm2_5 * 100;
	cell->pm_ugmx100[2] = current_readings.sen5x.pm4_0 * 100;
	cell->pm_ugmx100[3] = current_readings.sen5x.pm10_0 * 100;

	cell->voc_index = current_readings.sen5x.voc_index * 1; //x1
	cell->nox_index = current_readings.sen5x.nox_index * 1; //x1
}

void populate_next_log_cell()
{
	if (next_log_cell_nr == LOG_CELL_NR_UNKOWN)
	{
		next_log_cell_nr = flash_get_next_log_cell_nr();
	}

	if (next_log_cell_nr == LOG_CELL_NR_FULL || next_log_cell_nr > (MAX_LOG_CELL_COUNT - 10))
	{
		return;
	}

	struct flash_log_cell cell;
	populate_log_cell(&cell);
	flash_write_cell_by_nr(next_log_cell_nr, &cell);

	next_log_cell_nr++;
}
