
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(misc, LOG_LEVEL_DBG);

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#include <hal/nrf_gpio.h>


static const struct gpio_dt_spec pin_buck_enable =
	GPIO_DT_SPEC_GET(DT_NODELABEL(cat_misc), buck_enable_gpios);
static const struct gpio_dt_spec pin_lcd_backlight =
	GPIO_DT_SPEC_GET(DT_NODELABEL(cat_misc), lcd_backlight_gpios);
static const struct gpio_dt_spec pin_speaker =
	GPIO_DT_SPEC_GET(DT_NODELABEL(cat_misc), speaker_gpios);

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