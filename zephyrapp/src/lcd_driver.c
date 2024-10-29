
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/display.h>

#include "lcd_driver.h"
#include "display_pinctrl.h"
#include "power_control.h"
#include "misc.h"
#include "buttons.h"

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

	display_write(display_dev, x, y, &desc, buffer);
}

uint16_t lcd_framebuffer[LCD_IMAGE_PIXELS] = {0x0};

uint16_t lcd_tile[32*32];

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

#define guint8 uint8_t
#define guint int
#include "img.c"

extern char font8x8_basic[128][8];

void lcd_write_char(int x, int y, char c)
{
	for (int bx = 0; bx < 8; bx++)
	{
		for (int by = 0; by < 8; by++)
		{
			if (font8x8_basic[(int)c][by] & (1<<bx))
			{
				const int scale = 1;

				for (int sx = 0; sx < scale; sx++)
				{
					for (int sy = 0; sy < scale; sy++)
					{
						lcd_framebuffer[((x+bx)*scale)+sx + ((((y+by)*scale)+sy)*LCD_IMAGE_W)] = 0xffff;
					}
				}
			}
		}
	}
}

void lcd_write_str(int x, int y, char* str)
{
	int ox = x;
	while (*str)
	{
		lcd_write_char(x, y, *str);
		x += 8;
		if ((*str) == '\n')
		{
			x = ox;
			y += 8;
		}
		str++;
	}
}

void lcd_init()
{
	LOG_DBG("Init lcd...");

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

	// LOG_DBG("Write test pattern...");

	// for (int i = 0; i < 100; i++)
	// {
	// 	lcd_framebuffer[(LCD_IMAGE_W*i)+i] = 0xf000;
	// 	lcd_framebuffer[(LCD_IMAGE_W*i)+i+5] = 0xf000;
	// }

	// LOG_DBG("Write LCD...");

	lcd_flip();
	// LOG_DBG("Test write");

	// for (int p = 0; p < (sizeof(lcd_tile)/sizeof(lcd_tile[0])); p++)
	// {
	// 	lcd_tile[p] = 0b111111<<(6+5);
	// }

	// for (int i = 0; i < 4; i++)
	// {
	// 	lcd_blit(i*32, i*32, 32, 32, lcd_tile);
	// }

	LOG_DBG("Disable blanking...");

	display_blanking_off(display_dev);

	LOG_INF("Reached lcd_init done");


	int x = 75;
	int y = 100;
	int dx = 0;
	int step = 3;

	int last_epd = -30000;

	while (1)
	{
		// dx += step;

		// if (dx > 100 || dx < 0) step *= -1;

		memset(lcd_framebuffer, 0, sizeof(lcd_framebuffer));

		uint16_t transparent = (gimp_image.pixel_data[0] << 8) | (gimp_image.pixel_data[1] << 0);

		for (int px = 0; px < gimp_image.width; px++)
		{
			for (int py = 0; py < gimp_image.height; py++)
			{
				int offset = ((py * gimp_image.width) + px) * 2;
				uint16_t pixel = (gimp_image.pixel_data[offset] << 8) | (gimp_image.pixel_data[offset+1] << 0);

				if (pixel != transparent)
				{
					lcd_framebuffer[((y+py+dx)*LCD_IMAGE_W) + (x+px)] = pixel;
				}
			}
		}

		

		uint8_t buttons = get_buttons();
		// LOG_DBG("Buttons: %02x", buttons);

		char buf[256] = {0};
		snprintf(buf, 256, 
			"CAT Test\nButtons: %02x\nUptime: %lldms\n\n(:3)",
			buttons, k_uptime_get());
		// LOG_DBG("write '%s'", buf);
		lcd_write_str(0, 0, buf);

		if (buttons & CAT_BTN_MASK_UP) y -= step;
		if (buttons & CAT_BTN_MASK_DOWN) y += step;
		if (buttons & CAT_BTN_MASK_LEFT) x -= step;
		if (buttons & CAT_BTN_MASK_RIGHT) x += step;
		if (buttons & CAT_BTN_MASK_START)
		{
			epaper_render_test();
			pc_set_mode(true);
		}
		if (buttons & CAT_BTN_MASK_SELECT)
		{
			test_speaker();
		}

		lcd_flip();

		// if ((k_uptime_get() - last_epd) > 30000)
		// {
		// 	epaper_render_test();
		// 	pc_set_mode(true);
		// 	last_epd = k_uptime_get();
		// }
	}
}

