#include <zephyr/kernel.h>
#include <zephyr/drivers/flash.h>
#include <stdint.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(cat_flash, LOG_LEVEL_DBG);

#include "airquality.h"
#include "rtc.h"
#include "flash.h"
#include "sdcard.h"

#define SECTOR_SIZE 4096

#define FLASH_MAGIC 0xeebeefee

struct __attribute__((__packed__)) flash_header {
	uint32_t magic_number;
	uint32_t nrf70_fw_offset;
	uint32_t nrf70_fw_size;
	uint32_t persistance_offset;
} flash_header_read = {0};

static inline bool cell_is_valid(struct flash_log_cell* cell)
{
	return cell->timestamp != 0 && cell->timestamp != UINT64_MAX;
}

#define FLASH_SIZE 0x1000000
#define PERSISTANCE_OFFSET (flash_header_read.persistance_offset + TOTAL_SAVE_ROOM)
#define SYSTEM_OFFSET (flash_header_read.persistance_offset)
#define TOMAS_OFFSET (flash_header_read.persistance_offset+ROOM_FOR_SYSTEM)
#define MAX_LOG_CELL_COUNT (((FLASH_SIZE - PERSISTANCE_OFFSET) / sizeof(struct flash_log_cell)) & ~1)

#define LOG_CELL_NR_FULL -1
#define LOG_CELL_NR_UNKOWN -2

int next_log_cell_nr = LOG_CELL_NR_UNKOWN;
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

void flash_save_tomas_save(uint8_t* buf, size_t size)
{
	size += 0x1000 - (size % 0x1000);

	if (size > ROOM_FOR_TOMAS)
	{
		LOG_ERR("uhhhhhh, too big?");
		return;
	}

	flash_erase(flash_dev, TOMAS_OFFSET, size);
	flash_write(flash_dev, TOMAS_OFFSET, buf, size);
}

void flash_load_tomas_save(uint8_t* buf, size_t size)
{
	flash_read(flash_dev, TOMAS_OFFSET, buf, size);
}

void flash_nuke_tomas_save()
{
	flash_erase(flash_dev, TOMAS_OFFSET, 0x1000);
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
	cell->flags = 0xff;

	int now = k_uptime_get();
	int ago[3] = {
		current_readings.lps22hh.uptime_last_updated,
		current_readings.sunrise.uptime_last_updated,
		current_readings.sen5x.uptime_last_updated
	};

	int longest_ago = 0;

	for (int i = 0; i < 3; i++)
	{
		int past = now - ago[i];

		// LOG_DBG("ago[%d] = %d; past = %d", i, ago[i], past);

		if (past > longest_ago)
			longest_ago = past;
	}

	// LOG_DBG("longest_ago = %d", longest_ago);

	longest_ago += 1000 - (longest_ago % 1000);

	int ago_offset = longest_ago/1000;

	// LOG_DBG("rounded = %d; offset = %d", longest_ago, ago_offset);

	cell->timestamp = get_current_rtc_time() - ago_offset; // epoch time timestamp in seconds

	// LOG_DBG("cell->timestamp = %d", cell->timestamp);

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

void flash_erase_all_cells()
{
	int next = flash_get_next_log_cell_nr();
	int end = OFFSET_OF_CELL(next);
	end += 0x1000 - (end % 0x1000);

	int res = flash_erase(flash_dev, PERSISTANCE_OFFSET, end);
	if (res)
	{
		LOG_ERR("flash_erase_all_cells failed: %d", res);
		return;
	}

	next_log_cell_nr = 0;
}

bool is_ready_for_aqi_logging()
{
	return 
			// current_readings.lps22hh.uptime_last_updated &&
		   current_readings.sunrise.uptime_last_updated &&
		   current_readings.sen5x.uptime_last_updated;
}


#include <zephyr/shell/shell.h>

static int sh_retrieve_cell(const struct shell *sh, size_t argc,
				   char **argv)
{
	int nr = atoi(argv[1]);

	struct flash_log_cell cell;
	flash_get_cell_by_nr(nr, &cell);

	printf("Cell[%d] = {\n", nr);
	printf("\t.flags = 0x%02x\n", cell.flags);
	printf("\t.timestamp = %lld\n", cell.timestamp);
	printf("\t.temp_Cx1000 = %d\n", cell.temp_Cx1000);
	printf("\t.rh_pctx100 = %d\n", cell.rh_pctx100);
	printf("\t.co2_ppmx1 = %d\n", cell.co2_ppmx1);
	printf("\t.pm_ugmx100 = {%d, %d, %d, %d}\n", cell.pm_ugmx100[0], cell.pm_ugmx100[1], cell.pm_ugmx100[2], cell.pm_ugmx100[3]);
	printf("\t.voc_index = %d\n", cell.voc_index);
	printf("\t.nox_index = %d\n", cell.nox_index);
	printf("}\n");

	return 0;
}

static int sh_erase_log(const struct shell *sh, size_t argc,
				   char **argv)
{
	flash_erase_all_cells();
	return 0;
}

static int sh_fetch_next(const struct shell *sh, size_t argc,
				   char **argv)
{
	next_log_cell_nr = flash_get_next_log_cell_nr();
	printf("next_log_cell_nr = %d\n", next_log_cell_nr);
	return 0;
}

static int sh_get_time(const struct shell *sh, size_t argc,
				   char **argv)
{
	printf("get_current_rtc_time() = %lld\n", get_current_rtc_time());
	printf(" -> epoch = %lld\n", RTC_TIME_TO_EPOCH_TIME(get_current_rtc_time()));
	return 0;
}

static int sh_write_out(const struct shell *sh, size_t argc,
				   char **argv)
{
	write_log_to_sdcard();
	return 0;
}

static int sh_erasegame(const struct shell *sh, size_t argc,
				   char **argv)
{
	flash_nuke_tomas_save();
	return 0;
}


SHELL_STATIC_SUBCMD_SET_CREATE(sub_log,
	SHELL_CMD_ARG(retrieve, NULL,
		"Get cell",
		sh_retrieve_cell, 2, 0),
	SHELL_CMD_ARG(erase, NULL,
		"Erase whole log",
		sh_erase_log, 1, 0),
	SHELL_CMD_ARG(fetchnext, NULL,
		"Fetch next",
		sh_fetch_next, 1, 0),
	SHELL_CMD_ARG(gettime, NULL,
		"Get time",
		sh_get_time, 1, 0),
	SHELL_CMD_ARG(writeout, NULL,
		"Write log to SD card",
		sh_write_out, 1, 0),
	SHELL_CMD_ARG(erasegame, NULL,
		"Erase game state",
		sh_erasegame, 1, 0),
	SHELL_SUBCMD_SET_END /* Array terminated. */
);

SHELL_CMD_REGISTER(log, &sub_log, "Flash logs", NULL);

