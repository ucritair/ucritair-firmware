
void epaper_render_test();
extern bool epaper_flip_y;

struct epaper_image_asset {
	uint8_t w, h, stride;
	uint8_t bytes[];
};

extern struct epaper_image_asset epaper_image_cloud_default;
extern struct epaper_image_asset epaper_image_cloud_happy;
extern struct epaper_image_asset epaper_image_cloud_sad;
extern struct epaper_image_asset epaper_image_cloud_smoke;

extern struct epaper_image_asset epaper_image_unicorn_default;
extern struct epaper_image_asset epaper_image_unicorn_happy;
extern struct epaper_image_asset epaper_image_unicorn_mask;
extern struct epaper_image_asset epaper_image_unicorn_sad;
