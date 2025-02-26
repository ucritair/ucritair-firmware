
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/display.h>
#include <zephyr/pm/device.h>

#include "lcd_driver.h"
#include "display_pinctrl.h"
#include "power_control.h"
#include "misc.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(lcd_driver, LOG_LEVEL_DBG);

// static const struct gpio_dt_spec pin_lcd_backlight =
// 	GPIO_DT_SPEC_GET(DT_NODELABEL(cat_misc), lcd_backlight_gpios);

static const struct pwm_dt_spec pwm_led0 = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));

const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

extern uint32_t hack_cyc_before_data_write, hack_cyc_after_data_write, hack_before_blit, hack_after_blit;

volatile bool write_done = true;

struct mipi_dbi_spi_data {
	struct k_mutex lock;
	/* Used for 3 wire mode */
	uint16_t spi_byte;
};

void mipi_dbi_write_done_callback(const struct device *dev, int result, void *unused)
{
	struct mipi_dbi_spi_data *data = dev->data;
	k_mutex_unlock(&data->lock);
	write_done=true;

	if (result)
	{
		LOG_ERR("mipi_dbi_write_done_callback: result=%d", result);
	}
}

void lcd_blit(int x, int y, int w, int h, uint16_t* buffer)
{
	while (!write_done) {}

	struct display_buffer_descriptor desc = {
		.buf_size = w*h,
		.width = w,
		.height = h,
		.pitch = w
	};

	hack_before_blit = k_cycle_get_32();
	
	display_write(display_dev, x, y, &desc, buffer);
	hack_after_blit = k_cycle_get_32();
}

#ifdef LCD_FRAMEBUFFER_A_B
uint16_t lcd_framebuffer_pair[LCD_FRAMEBUFFER_PIXELS][2];
uint16_t* lcd_framebuffer = lcd_framebuffer_pair[0];
#else
uint16_t lcd_framebuffer_backing[LCD_FRAMEBUFFER_PIXELS];
uint16_t* lcd_framebuffer = lcd_framebuffer_backing;
#endif

int framebuffer_offset_h = 0;

void lcd_flip(uint16_t* buffer, int offset)
{
	lcd_blit(0, offset, LCD_IMAGE_W, LCD_FRAMEBUFFER_H, buffer);
}

void set_backlight(int pct)
{
	// if (!is_3v3_on)
	// {
	// 	LOG_ERR("Backlight w/o 3v3 is meaningless!!");
	// 	k_panic();
	// }

	// init_pin(&pin_lcd_backlight, "pin_lcd_backlight", GPIO_OUTPUT_INACTIVE);
	// // nrf_gpio_pin_control_select(NRF_GPIO_PIN_MAP(1, pin_lcd_backlight.pin), NRF_GPIO_PIN_SEL_APP);
	// LOG_DBG("backlight turn on");
	// pin_write(&pin_lcd_backlight, true);

	// enum pm_device_state state;
	// if (pm_device_state_get(pwm_led0.dev, &state))
	// {
	// 	LOG_ERR("Failed to get state");
	// }

	// if (pct == 0)
	// {
	// 	if (state != PM_DEVICE_STATE_SUSPENDED)
	// 	{
	// 		LOG_INF("Invoke PM_DEVICE_ACTION_SUSPEND");
	// 		pm_device_action_run(pwm_led0.dev, PM_DEVICE_ACTION_SUSPEND);
	// 	}
	// }
	// else
	// {
	// 	if (state == PM_DEVICE_STATE_SUSPENDED)
	// 	{
	// 		LOG_INF("Invoke PM_DEVICE_ACTION_RESUME");
	// 		pm_device_action_run(pwm_led0.dev, PM_DEVICE_ACTION_RESUME);
	// 	}
		
	// }

	pwm_set_dt(&pwm_led0, 1e+6, pct*1e+4);
}

void lcd_init()
{
	LOG_DBG("Init lcd...");

	LOG_INF("lcd_framebuffer=%p", lcd_framebuffer);

	set_backlight(BACKLIGHT_FULL);
	pc_set_mode(true);

	if (!device_is_ready(display_dev)) {
		LOG_ERR("Device %s not ready. Aborting sample.",
			display_dev->name);
		while (1)
		{
			LOG_ERR("dev not ready");
			k_msleep(1000);
		}
		return;
	}
	// lcd_flip();

	LOG_DBG("Disable blanking...");

	display_blanking_off(display_dev);

	LOG_INF("Reached lcd_init done");
	
}

