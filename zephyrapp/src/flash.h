#include "cat_core.h"

void test_flash();

extern bool did_post_flash;

#define TOTAL_SAVE_ROOM 0x10000
#define ROOM_FOR_TOMAS  0x0e000
#define ROOM_FOR_SYSTEM 0x02000

#if (ROOM_FOR_TOMAS + ROOM_FOR_SYSTEM) != TOTAL_SAVE_ROOM
#error adjust
#endif

int flash_get_nrf70_fw_size(int* size);
int flash_load_nrf70_fw(uint8_t* target, uint8_t** fw_start, uint8_t** fw_end);

extern int next_log_cell_nr;
void populate_next_log_cell();
void flash_get_cell_by_nr(int nr, CAT_log_cell* out);
int flash_get_first_cell_before_time(int memo_start_before, uint64_t t, CAT_log_cell* cell);
int flash_get_first_cell_after_time(int memo_start_after, uint64_t t, CAT_log_cell* cell);
int flash_get_first_calendar_cell(CAT_log_cell* cell);
void populate_log_cell(CAT_log_cell* cell);
void flash_erase_all_cells();
bool is_ready_for_aqi_logging();

float get_hours_of_logging_at_rate(int rate);

void flash_save_tomas_save(uint8_t* buf, size_t size);
void flash_load_tomas_save(uint8_t* buf, size_t size);
void flash_nuke_tomas_save();
