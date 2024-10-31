#include "lcd_rendering.h"
#include "buttons.h"
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <zephyr/kernel.h>
#include "airquality.h"
#include "sound.h"
#include "rgb_leds.h"
#include "epaper_rendering.h"
#include "touch.h"

typedef void (*menu_t)();
typedef void (*menu_op_t)(void*);

void menu_root();

menu_t current_menu = &menu_root;
int draw_y = 0;
int curr_idx = 0;
int selected_idx = -1;

bool selected = false;

uint8_t last_buttons = 0;

extern bool in_debug_menu, show_fps;

static void text(char* text)
{
	lcd_write_str(0xffff, 0, draw_y, text);
	draw_y += 8;
}

static void selectable(char* text, menu_op_t op, void* arg)
{
	lcd_write_str(0xffe0, 0, draw_y, text);
	if (selected_idx == curr_idx)
	{
		lcd_write_str(0x001f, strlen(text)*8 + 8, draw_y, "<");
		if (selected && op)
		{
			op(arg);
		}
	}
	draw_y += 8;
	curr_idx += 1;
}

void goto_menu(void* arg)
{
	current_menu = (menu_t)arg;
	selected_idx = 0;
}

void exit_debug_menu(void* arg)
{
	in_debug_menu = false;
	k_msleep(50);
}

void menu_test_buzzer(void* arg)
{
	test_speaker();
}

void menu_test_eink(void* arg)
{
	epaper_render_test();
}

char textf_buf[256];
#define textf(...) snprintf(textf_buf, sizeof(textf_buf)-1, __VA_ARGS__); text(textf_buf);

void menu_sensors()
{
	text("~~SENSORS MENU~~");
	textf("Uptime: %lldms", k_uptime_get());

	text("");
	textf("LPS22HH @ %lldms", current_readings.lps22hh.uptime_last_updated);
	textf("Temp: %.1fC; Pressure: %.1f?", (double)current_readings.lps22hh.temp, (double)current_readings.lps22hh.pressure);

	text("");
	textf("Sunrise @ %lldms", current_readings.sunrise.uptime_last_updated);
	textf("CO2: %.0fppm", (double)current_readings.sunrise.ppm_filtered_compensated);

	text("");
	textf("SEN5x @ %lldms", current_readings.sen5x.uptime_last_updated);
	textf("PM1.0: %.1f; PM2.5: %.1f", (double)current_readings.sen5x.pm1_0, (double)current_readings.sen5x.pm2_5);
	textf("PM4.0: %.1f; PM10.0: %.1f", (double)current_readings.sen5x.pm4_0, (double)current_readings.sen5x.pm10_0);
	textf("Humidity: %.1f%%RH; Temp: %.1fC", (double)current_readings.sen5x.humidity_rhpct, (double)current_readings.sen5x.temp_degC);
	textf("VOC: %.1f; NOX: %.1f", (double)current_readings.sen5x.voc_index, (double)current_readings.sen5x.nox_index);

	text("");
	selectable("Back", goto_menu, menu_root);
}

void menu_set_leds(void* arg)
{
	set_all_same_color(*(struct led_rgb*)arg);
}

#include <zephyr/drivers/led_strip.h>
#define RGB(_r, _g, _b) { .r = (_r), .g = (_g), .b = (_b) }
struct led_rgb red = RGB(0xff, 0x00, 0x00);
struct led_rgb green = RGB(0x00, 0xff, 0x00);
struct led_rgb blue = RGB(0x00, 0x00, 0xff);
struct led_rgb white = RGB(0xff, 0xff, 0xff);
struct led_rgb off = RGB(0x00, 0x00, 0x00);
struct led_rgb r_g = RGB(0xff, 0xff, 0x00);
struct led_rgb g_b = RGB(0x00, 0xff, 0xff);
struct led_rgb r_b = RGB(0xff, 0x00, 0xff);

void menu_leds()
{
	text("~~LEDS MENU~~");
	selectable("White", menu_set_leds, &white);
	selectable("Red", menu_set_leds, &red);
	selectable("Green", menu_set_leds, &green);
	selectable("Blue", menu_set_leds, &blue);
	selectable("R+G", menu_set_leds, &r_g);
	selectable("G+B", menu_set_leds, &g_b);
	selectable("R+B", menu_set_leds, &r_b);
	selectable("Off", menu_set_leds, &off);

	text("");
	selectable("Back", goto_menu, menu_root);
}

void menu_touch()
{
	text("~~TOUCH MENU~~");
	textf("X: %5d Y: %5d Z: %5d", touch_mapped_x, touch_mapped_y, touch_pressure);

	lcd_write_char(0xff00, touch_mapped_x, touch_mapped_y, 'X');

	text("");
	selectable("Back", goto_menu, menu_root);
}

void menu_toggle_fps(void* arg)
{
	show_fps = !show_fps;
}

void menu_root()
{
	text("~~DEBUG MENU~~");
	selectable("Sensors", goto_menu, menu_sensors);
	selectable("Set RGB LEDs", goto_menu, menu_leds);
	selectable("Test Buzzer", menu_test_buzzer, NULL);
	selectable("Update eInk", menu_test_eink, NULL);
	selectable("Touch Diagnostics", goto_menu, menu_touch);
	selectable("Toggle show FPS", menu_toggle_fps, NULL);

	text("");
	selectable("Back to game", exit_debug_menu, NULL);
}

#define NEWLY_PRESSED(x) (new_buttons & x) && !(last_buttons & x)

void draw_debug_menu()
{
	draw_y = 0;
	curr_idx = 0;
	selected = false;

	uint8_t new_buttons = get_buttons();

	if (NEWLY_PRESSED(CAT_BTN_MASK_A))
	{
		selected = true;
	}

	if (NEWLY_PRESSED(CAT_BTN_MASK_UP))
	{
		selected_idx--;
	}

	if (NEWLY_PRESSED(CAT_BTN_MASK_DOWN))
	{
		selected_idx++;
	}

	last_buttons = new_buttons;

	current_menu();

	if (selected_idx < 0) selected_idx = curr_idx - 1;
	if (selected_idx == curr_idx) selected_idx = 0;
}
