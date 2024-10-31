
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(misc, LOG_LEVEL_DBG);

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#include <hal/nrf_gpio.h>

#include <zephyr/drivers/i2c.h>

#include <zephyr/drivers/led_strip.h>




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
	// nrf_gpio_pin_control_select(NRF_GPIO_PIN_MAP(1, pin->pin), NRF_GPIO_PIN_SEL_APP);
}



static const struct device* dev_i2c = DEVICE_DT_GET(DT_NODELABEL(arduino_i2c));

void test_i2c()
{
	while (!device_is_ready(dev_i2c)) {
        printf("waiting for i2c to be ready...\n");
        k_msleep(100);
    }

    k_msleep(1000);

    uint8_t dummy;
    i2c_read(dev_i2c, &dummy, 1, 0x68); // wake

    printf("i2c bus scan:\n");
    printf("    0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
    for (uint8_t y = 0; y <= 7; y++) {
        printf("%02x", y<<4);
        for (uint8_t x = 0; x <= 0xf; x++) {
            uint8_t addr = (y<<4) | x;
            uint8_t buf = 0;
            if (addr < 0x08 || addr > 0x77) {
                printf("   ");
            } else if (i2c_read(dev_i2c, &buf, 0, addr) == 0) {
                printf(" %02x", addr);
            } else {
                printf(" --");
            }
        }
        printf("\n");
    }

    uint8_t lps22_addr = 0x0F;
    uint8_t lps22_whoami = 0x00;

    if (i2c_write_read(dev_i2c, 0x5D, &lps22_addr, 1, &lps22_whoami, 1))
    {
    	LOG_ERR("LPS22 whoami failed");
    }
    else
    {
    	LOG_ERR("LPS22 whoami = %02x (b3 expected)", lps22_whoami);
    }

    uint8_t lis3_addr = 0x0F;
    uint8_t lis3_whoami = 0x00;

    if (i2c_write_read(dev_i2c, 0x19, &lis3_addr, 1, &lis3_whoami, 1))
    {
    	LOG_ERR("LIS3DH whoami failed");
    }
    else
    {
    	LOG_ERR("LIS3DH whoami = %02x (33 expected)", lis3_whoami);
    }

    uint8_t sen55_cmd[] = {0xD0, 0x14};
    uint8_t sen55_response[48] = {0};
    char sen55_model[33] = {0};

    if (i2c_write(dev_i2c, sen55_cmd, 2, 0x69))
    {
    	LOG_ERR("SEN55 cmd write failed");
    }
    else
    {
    	k_msleep(20);
	    if (i2c_read(dev_i2c, sen55_response, 48, 0x69))
	    {
	    	LOG_ERR("SEN55 whoami failed");
	    }
	    else
	    {
	    	int p = 0;
	    	for (int i = 0; i < 16; i++)
	    	{
	    		sen55_model[p++] = sen55_response[(i*3)];
	    		sen55_model[p++] = sen55_response[(i*3)+1];
	    	}
	    	LOG_HEXDUMP_DBG(sen55_response, 48, "sen55_response");
	    	LOG_ERR("SEN55 whoami = %s", sen55_model);
	    }
	}

	// pin_write(&pin_buck_enable, false);
	// pin_write(&pin_sen55_boost_enable, false);
	// k_msleep(1000);
	// pin_write(&pin_sen55_boost_enable, true);
	// k_msleep(100);
	// pin_write(&pin_buck_enable, true);
	// k_msleep(1000);

	if (i2c_write(dev_i2c, &dummy, 0, 0x68))
	{
		LOG_ERR("sunrise wake 1 errored");
	} // wake
	// if (i2c_write(dev_i2c, &dummy, 0, 0x68))
	// {
	// 	LOG_ERR("sunrise wake 1-1 errored");
	// } // wake

	k_msleep(1);

	uint8_t reset[2] = {0xa3, 0xff};
	if (i2c_write(dev_i2c, reset, 2, 0x68))
	{
		LOG_ERR("Sunrise reset failed");
	}

	k_msleep(15);

	if (i2c_write(dev_i2c, &dummy, 0, 0x68))
	{
		LOG_ERR("sunrise wake 1-2 errored");
	} // wake

	k_msleep(1);

	uint8_t sunrise_fwversion_cmd = 0x38;
	uint8_t sunrise_fwversion[2] = {0};

	if (i2c_write_read(dev_i2c, 0x68, &sunrise_fwversion_cmd, 1, sunrise_fwversion, 2))
    {
    	LOG_ERR("Sunrise fwver failed");
    }
    else
    {
    	LOG_HEXDUMP_DBG(sunrise_fwversion, 2, "sunrise_fwversion");
    	LOG_INF("sunrise fw=%d.%d", sunrise_fwversion[0], sunrise_fwversion[1]);
    }

    k_msleep(1);

    if (i2c_write(dev_i2c, &dummy, 0, 0x68))
	{
		LOG_ERR("sunrise wake 2 errored");
	} // wake

	uint8_t sunrise_temp_cmd = 0x08; // i2c addr
	uint8_t sunrise_tmp[2] = {0};

	k_msleep(1);

	if (i2c_write_read(dev_i2c, 0x68, &sunrise_temp_cmd, 1, sunrise_tmp, 2))
    {
    	LOG_ERR("Sunrise tmp failed");
    }
    else
    {
    	LOG_HEXDUMP_DBG(sunrise_tmp, 2, "sunrise_tmp");
    }

    k_msleep(1);


	if (i2c_write(dev_i2c, &dummy, 0, 0x68))
	{
		LOG_ERR("sunrise wake 3 errored");
	} // wake

    uint8_t sunrise_cmd = 0x70;
    uint8_t sunrise_response[17] = {0};

    k_msleep(1);

    if (i2c_write_read(dev_i2c, 0x68, &sunrise_cmd, 1, sunrise_response, 16))
    {
    	LOG_ERR("Sunrise whoami failed");
    }
    else
    {
    	sunrise_response[16] = 0;
    	LOG_HEXDUMP_DBG(sunrise_response, 16, "sunrise_repsponse");
    	LOG_ERR("Sunrise whoami = %s", sunrise_response);
    }


}
