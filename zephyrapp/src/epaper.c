
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(epaper, LOG_LEVEL_DBG);

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#include <hal/nrf_gpio.h>


static const struct gpio_dt_spec pin_sclk =
	GPIO_DT_SPEC_GET(DT_NODELABEL(epaper_display), sclk_gpios);
static const struct gpio_dt_spec pin_data =
	GPIO_DT_SPEC_GET(DT_NODELABEL(epaper_display), data_gpios);
static const struct gpio_dt_spec pin_csn =
	GPIO_DT_SPEC_GET(DT_NODELABEL(epaper_display), csn_gpios);
static const struct gpio_dt_spec pin_dc =
	GPIO_DT_SPEC_GET(DT_NODELABEL(epaper_display), dc_gpios);
static const struct gpio_dt_spec pin_rst =
	GPIO_DT_SPEC_GET(DT_NODELABEL(epaper_display), rst_gpios);
static const struct gpio_dt_spec pin_busy =
	GPIO_DT_SPEC_GET(DT_NODELABEL(epaper_display), busy_gpios);
static const struct gpio_dt_spec pin_buck_enable =
	GPIO_DT_SPEC_GET(DT_NODELABEL(epaper_display), buck_enable_gpios);
static const struct gpio_dt_spec pin_lcd_backlight =
	GPIO_DT_SPEC_GET(DT_NODELABEL(epaper_display), lcd_backlight_gpios);

// static const struct gpio_dt_spec pin_led0 =
// 	GPIO_DT_SPEC_GET(DT_NODELABEL(led0), gpios);

void pin_write(const struct gpio_dt_spec* pin, bool v)
{
	if (gpio_pin_set_dt(pin, v) < 0) LOG_ERR("Error in pin_write");
}

bool pin_read(const struct gpio_dt_spec* pin)
{
	int v = gpio_pin_get_dt(pin);
	if (v < 0) LOG_ERR("Error in pin_read");
	return v==1;
}

void pin_set_mode(const struct gpio_dt_spec* pin, bool is_output)
{
	if (is_output)
	{
		if (gpio_pin_configure_dt(pin, GPIO_OUTPUT_INACTIVE) < 0)
		{
			while (1) {LOG_ERR("Failed to reconfigure for output");}
		}
	}
	else
	{
		if (gpio_pin_configure_dt(pin, GPIO_INPUT | GPIO_PULL_UP) < 0)
		{
			while (1) {LOG_ERR("Failed to reconfigure for input");}
		}
	}
	nrf_gpio_pin_control_select(NRF_GPIO_PIN_MAP(1, pin->pin), NRF_GPIO_PIN_SEL_APP);
}

void std_sleep()
{
	uint32_t start = k_cycle_get_32();

	while ((k_cycle_get_32() - start) < ((sys_clock_hw_cycles_per_sec() / 1e9) * 500))
	{
		;
	}
}

void write_byte(uint8_t b)
{
	pin_write(&pin_csn, false);
	pin_set_mode(&pin_data, true);

	std_sleep();
	
	for (int i = 7; i >= 0; i--)
	{
		pin_write(&pin_sclk, false);
		pin_write(&pin_data, b & (1<<i));
		std_sleep();
		pin_write(&pin_sclk, true);
		std_sleep();
	}

	std_sleep();

	pin_write(&pin_csn, true);
	std_sleep();
}

uint8_t read_byte()
{
	pin_write(&pin_csn, false);
	pin_set_mode(&pin_data, false);

	std_sleep();

	uint8_t data = 0;
	
	for (int i = 7; i >= 0; i--)
	{
		pin_write(&pin_sclk, false);
		std_sleep();
		pin_write(&pin_sclk, true);
		std_sleep();
		data |= pin_read(&pin_data) << i;
	}

	std_sleep();

	pin_write(&pin_sclk, false);

	std_sleep();

	pin_write(&pin_csn, true);
	
	std_sleep();

	return data;
}

void send_command_opcode(uint8_t opcode)
{
	pin_write(&pin_dc, false);
	std_sleep();

	write_byte(opcode);

	std_sleep();
	pin_write(&pin_dc, true);
	std_sleep();
}

void send_write_command(uint8_t opcode, int len, uint8_t* data)
{
	send_command_opcode(opcode);	

	for (int i = 0; i < len; i++)
	{
		write_byte(data[i]);
	}
}

void send_read_command(uint8_t opcode, int len, uint8_t* data)
{
	send_command_opcode(opcode);

	k_msleep(10);

	for (int i = 0; i < len; i++)
	{
		data[i] = read_byte();
	}
}

void wait_for_ready(char* tag)
{
	k_msleep(10);
	while (1) {
		int val = pin_read(&pin_busy);
		if (val == 1) break;

		LOG_DBG("Spin waiting for busy '%s' (%d)", tag, val);
		k_msleep(1000);
	}
}

void cmd_poweron()
{
	// TODO: Turn on rail

	LOG_DBG("Power on device");

	k_msleep(5);

	pin_write(&pin_rst, true);
	k_msleep(5);
	pin_write(&pin_rst, false);
	k_msleep(5);
	pin_write(&pin_rst, true);
	k_msleep(5);

	wait_for_ready("cmd_poweron");

	// LOG_DBG("Issue 0x00 soft-reset");

	// send_write_command(0x00, 1, (uint8_t[]){0x0E});
	k_msleep(5);
}

uint8_t psr_data[2];

// Read out OTP data required later
// See: PDLS_EXT3_Basic_Fast/src/Screen_EPD_EXT3.cpp:COG_SmallKP_getDataOTP
// See: https://www.pervasivedisplays.com/wp-content/uploads/2023/02/ApplicationNote_Small_Size_wide-Temperature_EPD_v03_20231031_B.pdf sec 4
void cmd_read_psr_data()
{
	LOG_DBG("Issue 0xA2 read OTP");

	uint8_t bank_check[2];
	send_read_command(0xA2, 2, bank_check);

	// bank_check[0] is dummy
	// bank_check[1] is byte 0 (page ID)

	const int psr_skip = 0xB1B;
	const int bank_skip = 0x171B - psr_skip;

	// Depending on which of the two banks is active, skip various distances in to get 'PSR' data

	// COG_data = {cf, 02}

	LOG_DBG("OTP[0] = 0x%02x", bank_check[1]);

	if (bank_check[1] == 0xA5)
	{
		// Bank 0, nothing to do
		LOG_DBG("Bank0 already, OK");
	}
	else
	{
		LOG_DBG("Bank1, skipping...");

		// Bank 1
		for (int i = 1; i < bank_skip; i++) // Already read 1 byte
		{
			read_byte();

			if ((i % 0x100) == 0) LOG_DBG("Skip at 0x%03x", i);
		}

		uint8_t checkbyte = read_byte();

		LOG_DBG("After skipping to Bank 0, OTP[0] = 0x%02x", checkbyte);

		if (checkbyte != 0xA5)
		{
			while (1) {
				LOG_ERR("Failed to find Bank0");
				k_msleep(10000);
			}
		}
	}

	for (int i = 1; i < psr_skip; i++) // Already read 1 byte
	{
		read_byte();
	}

	psr_data[0] = read_byte();
	psr_data[1] = read_byte();
}

void cmd_initialize()
{
	// Set temperature
	send_write_command(0xE5, 1, (uint8_t[]){0x19}); // = 25C. TODO: Make come from temp sensor

	// Activate (?) temperature
	send_write_command(0xE0, 1, (uint8_t[]){0x02});

	// Input panel settings data
	send_write_command(0x00, 2, psr_data);
}

#define IMAGE_W 128
#define IMAGE_H 248
#define IMAGE_PX (IMAGE_W * IMAGE_H)
#define IMAGE_BYTES (IMAGE_PX / 8)

void cmd_write_image(uint8_t* image_data)
{
	LOG_DBG("Issue 0x10 write image");
	// Global update mode
	// DTM1 = New image
	send_write_command(0x10, IMAGE_BYTES, image_data);

	LOG_DBG("Issue 0x13 write dummy");

	// DTM2 = Dummy image
	send_write_command(0x13, 0, NULL);
	for (int i = 0; i < IMAGE_BYTES; i++)
	{
		write_byte(0x00);
	}
}

void cmd_update()
{
	LOG_DBG("Issue 0x04 (cmd_update/1)");

	send_write_command(0x04, 0, NULL);

	wait_for_ready("cmd_update/0x04");

	LOG_DBG("Issue 0x12 (cmd_update/2");

	send_write_command(0x12, 0, NULL);

	wait_for_ready("cmd_update/0x12");
}

void cmd_turn_off_dcdc()
{
	send_write_command(0x02, 0, NULL);

	wait_for_ready("cmd_turn_off_dcdc");

	// TODO: Turn off power
}

void cmd_turn_on_and_write(uint8_t* image)
{
	cmd_poweron();
	cmd_read_psr_data();
	cmd_initialize();
	cmd_write_image(image);
	cmd_update();
	cmd_turn_off_dcdc();
}

uint8_t test_image[IMAGE_BYTES] = {0};

void write_px(uint8_t* image, int x, int y, bool val)
{
	if (x > IMAGE_W || y > IMAGE_H) return;

	int pxcount = (y * IMAGE_W) + x;
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

void init_pin(const struct gpio_dt_spec* pin, char* name, gpio_flags_t flags)
{
	if (!gpio_is_ready_dt(pin))
	{
		LOG_ERR("%s not ready when init", name);
		while (1) {}
	}

	if (gpio_pin_configure_dt(pin, flags) < 0)
	{
		LOG_ERR("%s failed to configure", name);
		while (1) {}
	}
	nrf_gpio_pin_control_select(NRF_GPIO_PIN_MAP(1, pin->pin), NRF_GPIO_PIN_SEL_APP);
}

#define PIN_FOREACH_X(X)\
	X(pin_sclk, true)\
	X(pin_data, true)\
	X(pin_csn, true)\
	X(pin_dc, true)\
	X(pin_rst, true)\
	X(pin_busy, false)

void turn_on_backlight()
{
	init_pin(&pin_lcd_backlight, "pin_lcd_backlight", GPIO_OUTPUT_INACTIVE);
	// nrf_gpio_pin_control_select(NRF_GPIO_PIN_MAP(1, pin_lcd_backlight.pin), NRF_GPIO_PIN_SEL_APP);
	pin_write(&pin_lcd_backlight, true);
}

void turn_on_3v3()
{
	init_pin(&pin_buck_enable, "pin_buck_enable", GPIO_OUTPUT_INACTIVE);
	// nrf_gpio_pin_control_select(NRF_GPIO_PIN_MAP(1, pin_buck_enable.pin), NRF_GPIO_PIN_SEL_APP);
	pin_write(&pin_buck_enable, true);
}


void test_epaper()
{

	turn_on_3v3();

	// init_pin(&pin_led0, "led0", GPIO_OUTPUT_ACTIVE);
	// for (int i = 0; i < 40; i++)
	// {
	// 	LOG_INF("Wait %ds...", i);
	// 	k_msleep(100);
	// 	gpio_pin_toggle_dt(&pin_led0);
	// }

	#define X_INIT_PIN(p, output) \
		init_pin(&p, #p, output?GPIO_OUTPUT_INACTIVE:(GPIO_INPUT|GPIO_PULL_UP));\
		nrf_gpio_pin_control_select(NRF_GPIO_PIN_MAP(1, p.pin), NRF_GPIO_PIN_SEL_APP);
		// if (!output) nrf_gpio_cfg_input(NRF_GPIO_PIN_MAP(1, p.pin), NRF_GPIO_PIN_PULLUP);
		// nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(1, p.pin));
	PIN_FOREACH_X(X_INIT_PIN);

	// pin_set_mode(&pin_dc, true);

	#define X_DISPLAY_PIN(p, output) \
		LOG_DBG(#p ".port->name = '%s' (output=%d)", p.port->name, output); \
		LOG_DBG(#p ".pin = %d", p.pin); \
		LOG_DBG(#p ".flags = %d", p.dt_flags);
	PIN_FOREACH_X(X_DISPLAY_PIN)

	pin_write(&pin_csn, true);
	pin_write(&pin_rst, true);
	pin_write(&pin_dc, true);
	pin_write(&pin_buck_enable, true);

	// while (1)
	// {
	// 	#define X_TOGGLE_PIN(p, output) if (output) gpio_pin_toggle_dt(&p);
	// 	PIN_FOREACH_X(X_TOGGLE_PIN)

	// 	k_msleep(50);

	// 	LOG_DBG("Toggling... busy=%d", gpio_pin_get_dt(&pin_busy));
	// }

	for (int y = IMAGE_H/2; y < IMAGE_H; y += 5)
	{
		for (int x = 0; x < IMAGE_W; x++)
		{
			write_px(test_image, x, y, true);
		}
	}

	write_str(test_image, 0, 0, "Hello, World!\nEINK ON ZEPHYR\nNo arduino crap\n:3 :3 :3 :3");

	cmd_turn_on_and_write(test_image);
}