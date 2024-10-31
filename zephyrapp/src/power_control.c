
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>


#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(power_control, LOG_LEVEL_DBG);

#include "misc.h"
#include "power_control.h"

bool is_3v3_on, is_5v0_on, is_leds_on;

static const struct gpio_dt_spec pin_sen55_boost_enable =
	GPIO_DT_SPEC_GET(DT_NODELABEL(cat_misc), sen55_boost_enable_gpios);

static const struct gpio_dt_spec pin_buck_enable =
	GPIO_DT_SPEC_GET(DT_NODELABEL(cat_misc), buck_enable_gpios);

static const struct gpio_dt_spec pin_led_enable =
	GPIO_DT_SPEC_GET(DT_NODELABEL(cat_misc), led_enable_gpios);

void init_power_control()
{
	LOG_DBG("Init");
	init_pin(&pin_buck_enable, "pin_buck_enable", GPIO_OUTPUT_INACTIVE);
	init_pin(&pin_sen55_boost_enable, "pin_sen55_boost_enable", GPIO_OUTPUT_INACTIVE);
	init_pin(&pin_led_enable, "pin_led_enable", GPIO_OUTPUT_INACTIVE);

	is_3v3_on = false;
	is_5v0_on = false;
	is_leds_on = false;
}

void set_3v3(bool on)
{
	LOG_DBG("set_3v3(%d)", on);
	pin_write(&pin_buck_enable, on);
	k_msleep(10);
	is_3v3_on = on;
}

void set_5v0(bool on)
{
	LOG_DBG("set_5v0(%d)", on);
	pin_write(&pin_sen55_boost_enable, on);
	k_msleep(10);
	is_5v0_on = on;
}

void set_leds(bool on)
{
	LOG_DBG("set_leds(%d)", on);
	pin_write(&pin_led_enable, on);
	k_msleep(10);
	is_leds_on = on;
}
