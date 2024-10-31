
#include <zephyr/drivers/i2c.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(touch, LOG_LEVEL_DBG);

#include "lcd_driver.h"

int touch_mapped_x, touch_mapped_y, touch_pressure;

static const struct device* dev_i2c = DEVICE_DT_GET(DT_NODELABEL(arduino_i2c));

const int addr_ns2009 = 0x48;

uint16_t read_ns2009(uint8_t addr)
{
	uint8_t buf[2];
	if (i2c_write_read(dev_i2c, addr_ns2009, &addr, 1, buf, 2))
	{
		LOG_ERR("ns2009 read failed");
		return 0xffff;
	}

	return (buf[0] << 4) | (buf[1] >> 4);
}

const int ns2009_reg_X = 0x90;
const int ns2009_reg_Y = 0x80;
const int ns2009_reg_Z1 = 0xe0;

const int xmin = 300;
const int xmax = 3800;
const int ymin = 300;
const int ymax = 3800;

void touch_update()
{
	int x = read_ns2009(ns2009_reg_X);
	int y = read_ns2009(ns2009_reg_Y);
	int z = read_ns2009(ns2009_reg_Z1);

	// LOG_INF("NS2009: x=%5d y=%5d z=%d", x, y, z);

	touch_mapped_x = (x - xmin) / (xmax / LCD_IMAGE_W);
	touch_mapped_y = LCD_IMAGE_H - ((y - ymin) / (ymax / LCD_IMAGE_H));
	touch_pressure = z>50?z:0;
}
