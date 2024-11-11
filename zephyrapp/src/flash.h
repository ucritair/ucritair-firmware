
void test_flash();

extern bool did_post_flash;

struct __attribute__((__packed__)) flash_log_cell {
	uint8_t flags;
	uint8_t unused[3];

	uint64_t timestamp; // timestamp in ????
	int32_t temp_Cx1000; // C * 1000

	uint16_t rh_pctx100; // % * 100
	uint16_t co2_ppmx1; // ppm * 1
	uint16_t pm_ugmx100[4]; //1.0, 2.5, 4.0, 10.0 x100

	uint8_t voc_index, nox_index; //x1

	uint8_t pad[2];
};

int flash_get_nrf70_fw_size(int* size);
int flash_load_nrf70_fw(uint8_t* target, uint8_t** fw_start, uint8_t** fw_end);

extern int next_log_cell_nr;
void populate_next_log_cell();
void flash_get_cell_by_nr(int nr, struct flash_log_cell* out);
void populate_log_cell(struct flash_log_cell* cell);
bool is_ready_for_aqi_logging();
void do_aqi_log();