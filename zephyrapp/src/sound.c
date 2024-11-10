
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#include "misc.h"

static const struct gpio_dt_spec pin_speaker =
	GPIO_DT_SPEC_GET(DT_NODELABEL(cat_misc), speaker_gpios);

void test_speaker()
{
	init_pin(&pin_speaker, "pin_speaker", GPIO_OUTPUT_INACTIVE);

	for (int i = 0; i < 50; i++)
	{
		gpio_pin_toggle_dt(&pin_speaker);
		k_msleep(1);
	}
}
