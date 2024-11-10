
void test_flash();

extern bool did_post_flash;

int flash_get_nrf70_fw_size(int* size);
int flash_load_nrf70_fw(uint8_t* target, uint8_t** fw_start, uint8_t** fw_end);

extern int next_log_cell_nr;
void populate_next_log_cell();