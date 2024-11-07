
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

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

void init_buttons()
{
	for (int i = 0; i < N_ROWS; i++)
	{
		init_pin(&btn_rows[i], "row_x", GPIO_INPUT|GPIO_PULL_UP);
	}

	for (int i = 0; i < N_COLS; i++)
	{
		init_pin(&btn_cols[i], "col_x", GPIO_OUTPUT_ACTIVE);
		gpio_pin_set_dt(&btn_cols[i], true);
	}
}

void deinit_buttons()
{
	for (int i = 0; i < N_COLS; i++)
	{
		// init_pin(&btn_cols[i], "col_x", GPIO_DISCONNECTED);
		gpio_pin_set_dt(&btn_cols[i], false);
	}
}

uint8_t get_buttons()
{
	uint8_t bits = 0;

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

	return ~bits;
}
