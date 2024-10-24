
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(misc, LOG_LEVEL_DBG);

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#include <hal/nrf_gpio.h>

#include <zephyr/drivers/i2c.h>

#include <zephyr/drivers/led_strip.h>


static const struct gpio_dt_spec pin_buck_enable =
	GPIO_DT_SPEC_GET(DT_NODELABEL(cat_misc), buck_enable_gpios);
static const struct gpio_dt_spec pin_lcd_backlight =
	GPIO_DT_SPEC_GET(DT_NODELABEL(cat_misc), lcd_backlight_gpios);
static const struct gpio_dt_spec pin_speaker =
	GPIO_DT_SPEC_GET(DT_NODELABEL(cat_misc), speaker_gpios);
static const struct gpio_dt_spec pin_sen55_boost_enable =
	GPIO_DT_SPEC_GET(DT_NODELABEL(cat_misc), sen55_boost_enable_gpios);
static const struct gpio_dt_spec pin_led_enable =
	GPIO_DT_SPEC_GET(DT_NODELABEL(cat_misc), led_enable_gpios);

#define N_ROWS 4
static const struct gpio_dt_spec btn_rows[N_ROWS] =
{
	GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(cat_misc), btn_row_gpios, 0),
	GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(cat_misc), btn_row_gpios, 1),
	GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(cat_misc), btn_row_gpios, 2),
	GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(cat_misc), btn_row_gpios, 3),
};

#define N_COLS 2
static const struct gpio_dt_spec btn_cols[N_COLS] =
{
	GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(cat_misc), btn_col_gpios, 0),
	GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(cat_misc), btn_col_gpios, 1),
};

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
};

static struct led_rgb pixels[STRIP_NUM_PIXELS];

static const struct device *const strip = DEVICE_DT_GET(STRIP_NODE);


void init_pin(const struct gpio_dt_spec* pin, char* name, gpio_flags_t flags)
{
	if (!gpio_is_ready_dt(pin))
	{
		LOG_ERR("%s not ready when init", name);
		while (1) {}
	}

	if (gpio_pin_configure_dt(pin, flags) < 0)
	{
		LOG_ERR("%s failed to configure", name);
		while (1) {}
	}
	// nrf_gpio_pin_control_select(NRF_GPIO_PIN_MAP(1, pin->pin), NRF_GPIO_PIN_SEL_APP);
}

void turn_on_backlight()
{
	init_pin(&pin_lcd_backlight, "pin_lcd_backlight", GPIO_OUTPUT_INACTIVE);
	// nrf_gpio_pin_control_select(NRF_GPIO_PIN_MAP(1, pin_lcd_backlight.pin), NRF_GPIO_PIN_SEL_APP);
	pin_write(&pin_lcd_backlight, true);
}

void turn_on_3v3()
{
	init_pin(&pin_buck_enable, "pin_buck_enable", GPIO_OUTPUT_INACTIVE);
	// nrf_gpio_pin_control_select(NRF_GPIO_PIN_MAP(1, pin_buck_enable.pin), NRF_GPIO_PIN_SEL_APP);
	pin_write(&pin_buck_enable, true);
}

void turn_on_5v0()
{
	init_pin(&pin_sen55_boost_enable, "pin_sen55_boost_enable", GPIO_OUTPUT_INACTIVE);
	// nrf_gpio_pin_control_select(NRF_GPIO_PIN_MAP(1, pin_sen55_boost_enable.pin), NRF_GPIO_PIN_SEL_APP);
	pin_write(&pin_sen55_boost_enable, true);
}

void turn_on_leds()
{
	init_pin(&pin_led_enable, "pin_led_enable", GPIO_OUTPUT_INACTIVE);
	// nrf_gpio_pin_control_select(NRF_GPIO_PIN_MAP(1, pin_led_enable.pin), NRF_GPIO_PIN_SEL_APP);
	pin_write(&pin_led_enable, true);
}


void test_speaker()
{
	init_pin(&pin_speaker, "pin_speaker", GPIO_OUTPUT_INACTIVE);

	for (int i = 0; i < 200; i++)
	{
		gpio_pin_toggle_dt(&pin_speaker);
		k_msleep(1);
	}
}

void init_matrix()
{
	for (int i = 0; i < N_ROWS; i++)
	{
		init_pin(&btn_rows[i], "row_x", GPIO_INPUT);
	}

	for (int i = 0; i < N_COLS; i++)
	{
		init_pin(&btn_cols[i], "col_x", GPIO_OUTPUT_ACTIVE);
		gpio_pin_set_dt(&btn_cols[i], true);
	}
}

uint16_t scan_matrix()
{
	uint16_t bits = 0;

	for (int col = 0; col < N_COLS; col++)
	{
		gpio_pin_set_dt(&btn_cols[col], false);
		k_usleep(25);
		for (int row = 0; row < N_ROWS; row++)
		{
			bits <<= 1;
			bits |= gpio_pin_get_dt(&btn_rows[row]);
		}
		gpio_pin_set_dt(&btn_cols[col], true);
		k_usleep(25);
	}

	return bits;
}

static const struct device* dev_i2c = DEVICE_DT_GET(DT_NODELABEL(arduino_i2c));

void test_i2c()
{
	while (!device_is_ready(dev_i2c)) {
        printf("waiting for i2c to be ready...\n");
        k_msleep(100);
    }

    printf("i2c bus scan:\n");
    printf("    0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
    for (uint8_t y = 0; y <= 7; y++) {
        printf("%02x", y<<4);
        for (uint8_t x = 0; x <= 0xf; x++) {
            uint8_t addr = (y<<4) | x;
            uint8_t buf = 0;
            if (addr < 0x08 || addr > 0x77) {
                printf("   ");
            } else if (i2c_read(dev_i2c, &buf, 0, addr) == 0) {
                printf(" %02x", addr);
            } else {
                printf(" --");
            }
        }
        printf("\n");
    }
}

#if STRIP_NUM_PIXELS != 8
#error len wrong
#endif

void test_leds()
{
	size_t color = 0;
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
		return 0;
	}

	turn_on_leds();
	k_msleep(10);

	LOG_INF("Displaying pattern on strip");
	for (int loop = 0; loop < 6; loop++) {
		for (int i = 0; i < STRIP_NUM_PIXELS; i++)
		{
			pixels[i] = colors[loop%3];
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