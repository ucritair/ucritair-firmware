
#include <zephyr/kernel.h>
#include <zephyr/drivers/led_strip.h>
#include "power_control.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(rgb_leds, LOG_LEVEL_INF);

#define STRIP_NODE		DT_NODELABEL(led_strip)

#if DT_NODE_HAS_PROP(DT_NODELABEL(led_strip), chain_length)
#define STRIP_NUM_PIXELS	DT_PROP(DT_NODELABEL(led_strip), chain_length)
#else
#error Unable to determine length of LED strip
#endif

#define DELAY_TIME K_MSEC(100)

#define RGB(_r, _g, _b) { .r = (_r), .g = (_g), .b = (_b) }

static const struct led_rgb colors[] = {
	RGB(0xff, 0x00, 0x00), /* red */
	RGB(0x00, 0xff, 0x00), /* green */
	RGB(0x00, 0x00, 0xff), /* blue */
	RGB(0, 0, 0)
};

static struct led_rgb pixels[STRIP_NUM_PIXELS];

static const struct device *const strip = DEVICE_DT_GET(STRIP_NODE);


#if STRIP_NUM_PIXELS != 8
#error len wrong
#endif

void test_leds()
{
	int rc;

	// const struct device* i2s = DEVICE_DT_GET(DT_PROP(DT_NODELABEL(led_strip), i2s_dev));

	// if (device_is_ready(i2s)) {
	// 	LOG_INF("Found bakcing i2s strip device %s", i2s->name);
	// } else {
	// 	LOG_ERR("bakcing i2s strip device %s is not ready", i2s->name);
	// 	return 0;
	// }

	if (device_is_ready(strip)) {
		LOG_INF("Found LED strip device %s", strip->name);
	} else {
		LOG_ERR("LED strip device %s is not ready", strip->name);
		return;
	}

	set_leds(true);

	LOG_INF("Displaying pattern on strip");
	for (int loop = 0; loop < 8; loop++) {
		for (int i = 0; i < STRIP_NUM_PIXELS; i++)
		{
			pixels[i] = colors[loop%4];
			// pixels[i].r = 0xAA;
			// pixels[i].g = 0xAA;
			// pixels[i].b = 0xAA;

		}

		rc = led_strip_update_rgb(strip, pixels, STRIP_NUM_PIXELS);
		LOG_HEXDUMP_DBG(pixels, sizeof(pixels), "pixeldata");
		if (rc) {
			LOG_ERR("couldn't update strip: %d", rc);
		}

		k_sleep(DELAY_TIME);
	}
}

void set_all_same_color(struct led_rgb color)
{
	for (int i = 0; i < STRIP_NUM_PIXELS; i++)
	{
		pixels[i] = color;
	}

	set_leds(true);

	LOG_INF("set_all_same_color r=%d, g=%d, b=%d", color.r, color.g, color.b);
	int rc = led_strip_update_rgb(strip, pixels, STRIP_NUM_PIXELS);
	if (rc)
	{
		LOG_ERR("couldn't update strip: %d", rc);
	}
}