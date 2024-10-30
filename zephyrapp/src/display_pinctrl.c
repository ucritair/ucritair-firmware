
#include <zephyr/init.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pinctrl.h>
#include <hal/nrf_gpio.h>
#include <zephyr/kernel.h>



#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(display_pinctrl, LOG_LEVEL_DBG);


// TODO: This shit should be derived from devicetree

PINCTRL_DT_DEV_CONFIG_DECLARE(DT_NODELABEL(spi2));

// PINCTRL_DT_STATE_PINS_DEFINE(DT_NODELABEL(pinctrl), pc_alt_lcd);

// static const struct pinctrl_state pc_alt_lcd[] = {
// 	PINCTRL_DT_STATE_INIT(pc_alt_lcd, PINCTRL_STATE_DEFAULT),
// };

// PINCTRL_DT_STATE_PINS_DEFINE(DT_NODELABEL(pinctrl), pc_alt_epd);

// static const struct pinctrl_state pc_alt_epd[] = {
// 	PINCTRL_DT_STATE_INIT(pc_alt_epd, PINCTRL_STATE_DEFAULT),
// };

static const pinctrl_soc_pin_t pc_alt_lcd_pins[] = {
	NRF_PSEL(SPIM_SCK, 0, 8),
	NRF_PSEL_DISCONNECTED(SPIM_MISO),
	NRF_PSEL(SPIM_MOSI, 0, 9)
};

// static const struct pinctrl_state pc_alt_lcd = {
// 	.pins = pc_alt_lcd_pins,
// 	.pin_cnt = sizeof(pc_alt_lcd_pins)/sizeof(pc_alt_lcd_pins[0]),
// 	.id = 2
// };

static const pinctrl_soc_pin_t pc_alt_epd_pins[] = {
	NRF_PSEL_DISCONNECTED(SPIM_SCK),
	NRF_PSEL_DISCONNECTED(SPIM_MISO),
	NRF_PSEL_DISCONNECTED(SPIM_MOSI)
};

// static const struct pinctrl_state pc_alt_epd = {
// 	.pins = pc_alt_epd_pins,
// 	.pin_cnt = sizeof(pc_alt_epd_pins)/sizeof(pc_alt_epd_pins[0]),
// 	.id = 3
// };

void pc_set_mode(bool lcd)
{
	LOG_INF("pc_set_mode to %s", lcd?"lcd":"epd");
	void* dev_cfg = NRF_SPIM4;

	int e;

	if (lcd)
	{
		e = pinctrl_configure_pins(pc_alt_lcd_pins, 3, (uintptr_t)dev_cfg);
	}
	else
	{
		e = pinctrl_configure_pins(pc_alt_epd_pins, 3, (uintptr_t)dev_cfg);
	}

	if (e)
	{
		while (1)
		{
			LOG_ERR("Pincontrol reconfigure for %s failed: %d", lcd?"lcd":"epd", e);
			k_msleep(1000);
		}
	}
}