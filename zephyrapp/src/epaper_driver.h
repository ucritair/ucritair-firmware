#include <stdbool.h>

#define EPD_IMAGE_H 128
#define EPD_IMAGE_W 248
#define EPD_IMAGE_PX (EPD_IMAGE_W * EPD_IMAGE_H)
#define EPD_IMAGE_BYTES (EPD_IMAGE_PX / 8)

#define EPD_STD_SLEEP 120
#define EPD_RATE_NS_PER_CLOCK_PHASE 60

void init_epaper_enough_for_it_to_calm_the_fuck_down();
void pc_set_mode(bool lcd);

void cmd_turn_on_and_write(uint8_t* image);
void cmd_turn_on_and_write_fast(uint8_t* old, uint8_t* image);
