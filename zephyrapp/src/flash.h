
void test_flash();

extern bool did_post_flash;

struct __attribute__((__packed__)) flash_log_cell {
	uint8_t flags;
	uint8_t unused[3];

	uint64_t timestamp; // timestamp in ????
	int32_t temp_Cx1000; // C * 1000
	uint16_t pressure_hPax10; // hPa * 10

	uint16_t rh_pctx100; // % * 100
	uint16_t co2_ppmx1; // ppm * 1
	uint16_t pm_ugmx100[4]; //PM 1.0, 2.5, 4.0, 10.0 x100
	uint16_t pn_ugmx100[5]; //PN 0.5, 1.0, 2.5, 4.0, 10.0 x100

	uint8_t voc_index, nox_index; //x1

	uint8_t pad[22];
};

#define FLAG_HAS_TEMP_RH_PARTICLES 1
#define FLAG_HAS_CO2 2

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
void flash_get_cell_by_nr(int nr, struct flash_log_cell* out);
int flash_get_first_cell_before_time(int memo_start_before, uint64_t t, struct flash_log_cell* cell);
void populate_log_cell(struct flash_log_cell* cell);
void flash_erase_all_cells();
bool is_ready_for_aqi_logging();
void do_aqi_log();

float get_hours_of_logging_at_rate(int rate);

void flash_save_tomas_save(uint8_t* buf, size_t size);
void flash_load_tomas_save(uint8_t* buf, size_t size);
