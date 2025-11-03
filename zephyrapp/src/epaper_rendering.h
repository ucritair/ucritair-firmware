
#include "epaper_driver.h"

void epaper_render_test();
void epaper_render_protected_off();
extern bool epaper_flip_y;

extern volatile uint8_t framebuffer_fast_update_count;
extern volatile uint8_t epaper_framebuffer[EPD_IMAGE_BYTES];
