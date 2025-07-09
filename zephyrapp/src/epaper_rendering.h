
#include "epaper_driver.h"

void epaper_render_test();
void epaper_render_protected_off();
extern bool epaper_flip_y;

struct epaper_image_asset {
	uint8_t w, h, stride;
	uint8_t bytes[];
};

extern volatile uint8_t framebuffer_fast_update_count;
extern volatile uint8_t epaper_framebuffer[EPD_IMAGE_BYTES];

extern struct epaper_image_asset epaper_image_cloud_default;
extern struct epaper_image_asset epaper_image_cloud_happy;
extern struct epaper_image_asset epaper_image_cloud_sad;
extern struct epaper_image_asset epaper_image_cloud_smoke;

extern struct epaper_image_asset epaper_image_unicorn_default;
extern struct epaper_image_asset epaper_image_unicorn_happy;
extern struct epaper_image_asset epaper_image_unicorn_mask;
extern struct epaper_image_asset epaper_image_unicorn_sad;
extern struct epaper_image_asset epaper_image_unicorn_low_battery;

extern struct epaper_image_asset epaper_image_protected;
