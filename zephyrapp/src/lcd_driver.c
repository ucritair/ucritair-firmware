
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/display.h>

#include "lcd_driver.h"
#include "display_pinctrl.h"
#include "power_control.h"
#include "misc.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(lcd_driver, LOG_LEVEL_DBG);

static const struct gpio_dt_spec pin_lcd_backlight =
	GPIO_DT_SPEC_GET(DT_NODELABEL(cat_misc), lcd_backlight_gpios);

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

uint16_t lcd_framebuffer[LCD_IMAGE_PIXELS] = {0x0};

void lcd_flip()
{
	

	const int subwrites = 1;
	const int section_y = LCD_IMAGE_H/subwrites;
	const int section_px = LCD_IMAGE_W*section_y;

	for (int i = 0; i < subwrites; i++)
	{
		lcd_blit(0, i*section_y, LCD_IMAGE_W, section_y, &lcd_framebuffer[i*section_px]);
	}
}

void turn_on_backlight()
{
	if (!is_3v3_on)
	{
		LOG_ERR("Backlight w/o 3v3 is meaningless!!");
		k_panic();
	}

	init_pin(&pin_lcd_backlight, "pin_lcd_backlight", GPIO_OUTPUT_INACTIVE);
	// nrf_gpio_pin_control_select(NRF_GPIO_PIN_MAP(1, pin_lcd_backlight.pin), NRF_GPIO_PIN_SEL_APP);
	LOG_DBG("backlight turn on");
	pin_write(&pin_lcd_backlight, true);
}

void lcd_init()
{
	LOG_DBG("Init lcd...");

	LOG_INF("lcd_framebuffer=%p", lcd_framebuffer);

	turn_on_backlight();
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
	lcd_flip();

	LOG_DBG("Disable blanking...");

	display_blanking_off(display_dev);

	LOG_INF("Reached lcd_init done");
	
}
