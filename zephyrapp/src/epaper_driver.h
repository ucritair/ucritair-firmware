
#define EPD_IMAGE_W 128
#define EPD_IMAGE_H 248
#define EPD_IMAGE_PX (EPD_IMAGE_W * EPD_IMAGE_H)
#define EPD_IMAGE_BYTES (EPD_IMAGE_PX / 8)

#define EPD_RATE_NS_PER_CLOCK_PHASE 120

void test_epaper();
void pc_set_mode(bool lcd);
