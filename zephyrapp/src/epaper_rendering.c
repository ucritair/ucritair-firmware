#include <stdint.h>
#include <stdbool.h>

#include "epaper_driver.h"

void write_px(uint8_t* image, int x, int y, bool val)
{
	if (x > EPD_IMAGE_W || y > EPD_IMAGE_H) return;

	int pxcount = (y * EPD_IMAGE_W) + x;
	int bytecount = pxcount >> 3;
	int bitcount = pxcount & 0b111;

	if (val)
	{
		image[bytecount] |= 1<<(7-bitcount);
	}
	else
	{
		image[bytecount] &= ~(1<<(7-bitcount));
	}
}

#include "font8x8_basic.h"

void write_char(uint8_t* image, int x, int y, char c)
{
	for (int bx = 0; bx < 8; bx++)
	{
		for (int by = 0; by < 8; by++)
		{
			if (font8x8_basic[(int)c][by] & (1<<bx))
			{
				write_px(image, x+bx, y+by, 1);
			}
		}
	}
}

void write_str(uint8_t* image, int x, int y, char* str)
{
	int ox = x;
	while (*str)
	{
		write_char(image, x, y, *str);
		x += 8;
		if ((*str) == '\n')
		{
			x = ox;
			y += 8;
		}
		str++;
	}
}


uint8_t test_image[EPD_IMAGE_BYTES] = {0};

void epaper_render_test()
{
	for (int y = EPD_IMAGE_H/2; y < EPD_IMAGE_H; y += 5)
	{
		for (int x = 0; x < EPD_IMAGE_W; x++)
		{
			write_px(test_image, x, y, true);
		}
	}

	write_str(test_image, 0, 0, "Hello, World!\nEINK ON ZEPHYR\nNo arduino crap\n:3 :3 :3 :3");

	cmd_turn_on_and_write(test_image);
}