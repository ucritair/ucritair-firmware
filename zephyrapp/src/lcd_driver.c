
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/display.h>

#include "lcd_driver.h"
#include "display_pinctrl.h"
#include "misc.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(lcd_driver, LOG_LEVEL_DBG);

static const struct gpio_dt_spec pin_lcd_backlight =
	GPIO_DT_SPEC_GET(DT_NODELABEL(cat_misc), lcd_backlight_gpios);

const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

void lcd_blit(int x, int y, int w, int h, uint16_t* buffer)
{
	struct display_buffer_descriptor desc = {
		.buf_size = w*h,
		.width = w,
		.height = h,
		.pitch = w
	};

	pc_set_mode(true);

	display_write(display_dev, x, y, &desc, buffer);
}

uint16_t lcd_framebuffer[LCD_IMAGE_PIXELS] = {0xffff};

uint16_t lcd_tile[32*32];

void lcd_flip()
{
	lcd_blit(0, 0, LCD_IMAGE_W, LCD_IMAGE_H, lcd_framebuffer);
}

void turn_on_backlight()
{
	init_pin(&pin_lcd_backlight, "pin_lcd_backlight", GPIO_OUTPUT_INACTIVE);
	// nrf_gpio_pin_control_select(NRF_GPIO_PIN_MAP(1, pin_lcd_backlight.pin), NRF_GPIO_PIN_SEL_APP);
	pin_write(&pin_lcd_backlight, true);
}

void lcd_init()
{
	LOG_DBG("Init lcd...");

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

	// LOG_DBG("Write test pattern...");

	// for (int i = 0; i < 100; i++)
	// {
	// 	lcd_framebuffer[(LCD_IMAGE_W*i)+i] = 0x00;
	// }

	// LOG_DBG("Write LCD...");

	// lcd_flip();
	LOG_DBG("Test write");

	memset(lcd_tile, sizeof(lcd_tile), 0xf0);

	for (int i = 0; i < 8; i++)
	{
		lcd_blit(i*32, i*32, 32, 32, lcd_tile);
	}

	LOG_DBG("Disable blanking...");

	display_blanking_off(display_dev);

	LOG_DBG("Turn on backlight...");

	turn_on_backlight();

	LOG_INF("Reached lcd_init done");
}

