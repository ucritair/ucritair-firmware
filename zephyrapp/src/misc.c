
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(misc, LOG_LEVEL_DBG);

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#include <hal/nrf_gpio.h>

#include <zephyr/drivers/i2c.h>

#include <zephyr/drivers/led_strip.h>


static const struct gpio_dt_spec pin_buck_enable =
	GPIO_DT_SPEC_GET(DT_NODELABEL(cat_misc), buck_enable_gpios);
static const struct gpio_dt_spec pin_lcd_backlight =
	GPIO_DT_SPEC_GET(DT_NODELABEL(cat_misc), lcd_backlight_gpios);
static const struct gpio_dt_spec pin_speaker =
	GPIO_DT_SPEC_GET(DT_NODELABEL(cat_misc), speaker_gpios);
static const struct gpio_dt_spec pin_sen55_boost_enable =
	GPIO_DT_SPEC_GET(DT_NODELABEL(cat_misc), sen55_boost_enable_gpios);
static const struct gpio_dt_spec pin_led_enable =
	GPIO_DT_SPEC_GET(DT_NODELABEL(cat_misc), led_enable_gpios);

#define N_ROWS 4
static const struct gpio_dt_spec btn_rows[N_ROWS] =
{
	GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(cat_misc), btn_row_gpios, 0),
	GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(cat_misc), btn_row_gpios, 1),
	GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(cat_misc), btn_row_gpios, 2),
	GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(cat_misc), btn_row_gpios, 3),
};

#define N_COLS 2
static const struct gpio_dt_spec btn_cols[N_COLS] =
{
	GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(cat_misc), btn_col_gpios, 0),
	GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(cat_misc), btn_col_gpios, 1),
};

#define STRIP_NODE		DT_NODELABEL(led_strip)

#if DT_NODE_HAS_PROP(DT_NODELABEL(led_strip), chain_length)
#define STRIP_NUM_PIXELS	DT_PROP(DT_NODELABEL(led_strip), chain_length)
#else
#error Unable to determine length of LED strip
#endif

#define DELAY_TIME K_MSEC(100)

#define RGB(_r, _g, _b) { .r = (_r), .g = (_g), .b = (_b) }

static const struct led_rgb colors[] = {
	RGB(0xff, 0x00, 0x00), /* red */
	RGB(0x00, 0xff, 0x00), /* green */
	RGB(0x00, 0x00, 0xff), /* blue */
	RGB(0, 0, 0)
};

static struct led_rgb pixels[STRIP_NUM_PIXELS];

static const struct device *const strip = DEVICE_DT_GET(STRIP_NODE);


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

void turn_on_5v0()
{
	init_pin(&pin_sen55_boost_enable, "pin_sen55_boost_enable", GPIO_OUTPUT_INACTIVE);
	// nrf_gpio_pin_control_select(NRF_GPIO_PIN_MAP(1, pin_sen55_boost_enable.pin), NRF_GPIO_PIN_SEL_APP);
	pin_write(&pin_sen55_boost_enable, true);
}

void turn_on_leds()
{
	init_pin(&pin_led_enable, "pin_led_enable", GPIO_OUTPUT_INACTIVE);
	// nrf_gpio_pin_control_select(NRF_GPIO_PIN_MAP(1, pin_led_enable.pin), NRF_GPIO_PIN_SEL_APP);
	pin_write(&pin_led_enable, true);
}


void test_speaker()
{
	init_pin(&pin_speaker, "pin_speaker", GPIO_OUTPUT_INACTIVE);

	for (int i = 0; i < 200; i++)
	{
		gpio_pin_toggle_dt(&pin_speaker);
		k_msleep(1);
	}
}

void init_matrix()
{
	for (int i = 0; i < N_ROWS; i++)
	{
		init_pin(&btn_rows[i], "row_x", GPIO_INPUT);
	}

	for (int i = 0; i < N_COLS; i++)
	{
		init_pin(&btn_cols[i], "col_x", GPIO_OUTPUT_ACTIVE);
		gpio_pin_set_dt(&btn_cols[i], true);
	}
}

uint16_t scan_matrix()
{
	uint16_t bits = 0;

	for (int col = 0; col < N_COLS; col++)
	{
		gpio_pin_set_dt(&btn_cols[col], false);
		k_usleep(25);
		for (int row = 0; row < N_ROWS; row++)
		{
			bits <<= 1;
			bits |= gpio_pin_get_dt(&btn_rows[row]);
		}
		gpio_pin_set_dt(&btn_cols[col], true);
		k_usleep(25);
	}

	return bits;
}

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

void report_ns2009()
{
	return;
	int x = read_ns2009(ns2009_reg_X);
	int y = read_ns2009(ns2009_reg_Y);
	int z = read_ns2009(ns2009_reg_Z1);

	LOG_INF("NS2009: x=%5d y=%5d z=%d", x, y, z);

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
}

#if STRIP_NUM_PIXELS != 8
#error len wrong
#endif

void test_leds()
{
	size_t color = 0;
	int rc;

	// const struct device* i2s = DEVICE_DT_GET(DT_PROP(DT_NODELABEL(led_strip), i2s_dev));

	// if (device_is_ready(i2s)) {
	// 	LOG_INF("Found bakcing i2s strip device %s", i2s->name);
	// } else {
	// 	LOG_ERR("bakcing i2s strip device %s is not ready", i2s->name);
	// 	return 0;
	// }

	if (device_is_ready(strip)) {
		LOG_INF("Found LED strip device %s", strip->name);
	} else {
		LOG_ERR("LED strip device %s is not ready", strip->name);
		return 0;
	}

	turn_on_leds();
	k_msleep(10);

	LOG_INF("Displaying pattern on strip");
	for (int loop = 0; loop < 8; loop++) {
		for (int i = 0; i < STRIP_NUM_PIXELS; i++)
		{
			pixels[i] = colors[loop%4];
			// pixels[i].r = 0xAA;
			// pixels[i].g = 0xAA;
			// pixels[i].b = 0xAA;

		}

		rc = led_strip_update_rgb(strip, pixels, STRIP_NUM_PIXELS);
		LOG_HEXDUMP_DBG(pixels, sizeof(pixels), "pixeldata");
		if (rc) {
			LOG_ERR("couldn't update strip: %d", rc);
		}

		k_sleep(DELAY_TIME);
	}
}

#include <stdio.h>
#include <string.h>
#include <zephyr/drivers/adc.h>

const struct device *adc_dev = DEVICE_DT_GET(DT_NODELABEL(adc));

#include <hal/nrf_saadc.h>
#define ADC_DEVICE_NAME DT_ADC_0_NAME
#define ADC_RESOLUTION 10
#define ADC_GAIN ADC_GAIN_1_4
#define ADC_REFERENCE ADC_REF_INTERNAL
#define ADC_ACQUISITION_TIME ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 10)
#define ADC_1ST_CHANNEL_ID 0
#define ADC_1ST_CHANNEL_INPUT NRF_SAADC_INPUT_AIN0
#define ADC_2ND_CHANNEL_ID 2
#define ADC_2ND_CHANNEL_INPUT NRF_SAADC_INPUT_AIN2

static const struct adc_channel_cfg m_1st_channel_cfg = {
	.gain = ADC_GAIN,
	.reference = ADC_REFERENCE,
	.acquisition_time = ADC_ACQUISITION_TIME,
	.channel_id = ADC_1ST_CHANNEL_ID,
#if defined(CONFIG_ADC_CONFIGURABLE_INPUTS)
	.input_positive = ADC_1ST_CHANNEL_INPUT,
#endif
};

#define BUFFER_SIZE 8
static int16_t m_sample_buffer[BUFFER_SIZE];

const struct adc_sequence_options sequence_opts = {
	.interval_us = 0,
	.callback = NULL,
	.user_data = NULL,
	.extra_samplings = 7,
};

static int adc_sample(void)
{
	int ret;

	const struct adc_sequence sequence = {
		.options = &sequence_opts,
		.channels = BIT(ADC_1ST_CHANNEL_ID),
		.buffer = m_sample_buffer,
		.buffer_size = sizeof(m_sample_buffer),
		.resolution = ADC_RESOLUTION,
	};

	if (!adc_dev) {
		return -1;
	}

	ret = adc_read(adc_dev, &sequence);
	if (ret)
	{
		printk("ADC read err: %d\n", ret);
	}

	printk("Sample: %d\n", m_sample_buffer[0]);

	// /* Print the AIN0 values */
	// printk("ADC raw value: ");
	// for (int i = 0; i < BUFFER_SIZE; i++) {
	// 	printk("%03x ", m_sample_buffer[i]);
	// }

	// printf("\n");
	
	// printf("\n Measured voltage: ");
	// for (int i = 0; i < BUFFER_SIZE; i++) {
	// 	float adc_voltage = 0;
	// 	adc_voltage = (float)(((float)m_sample_buffer[i] / 1023.0f) *
	// 			      3600.0f);
	// 	printk("%f ",adc_voltage);
	// }
	// printk("\n");


	return ret;
}

int test_adc(void)
{
	int err;

	printk("nRF53 SAADC sampling AIN0 (P0.13)\n");

	// adc_dev = device_get_binding("ADC_0");
	if (!device_is_ready(adc_dev)) {
		while (1)
		{
			printk("device_get_binding ADC_0 failed\n");
			k_msleep(1000);
		}
	}
	err = adc_channel_setup(adc_dev, &m_1st_channel_cfg);
	if (err) {
		while (1)
		{
			k_msleep(1000);
			printk("Error in adc setup: %d\n", err);
		}
		
	}

	/* Trigger offset calibration
	 * As this generates a _DONE and _RESULT event
	 * the first result will be incorrect.
	 */
	NRF_SAADC_S->TASKS_CALIBRATEOFFSET = 1;
	while (1) {
		err = adc_sample();
		if (err) {
			printk("Error in adc sampling: %d\n", err);
		}
		k_sleep(K_MSEC(500));
	}
}

#include <ff.h>
#include <zephyr/storage/disk_access.h>
#include <zephyr/logging/log.h>
#include <zephyr/fs/fs.h>

/*
 *  Note the fatfs library is able to mount only strings inside _VOLUME_STRS
 *  in ffconf.h
 */
#define DISK_DRIVE_NAME "SD"
#define DISK_MOUNT_PT "/"DISK_DRIVE_NAME":"

static FATFS fat_fs;
/* mounting info */
static struct fs_mount_t mp = {
	.type = FS_FATFS,
	.fs_data = &fat_fs,
};

#define FS_RET_OK FR_OK


#define MAX_PATH 128
#define SOME_FILE_NAME "some.dat"
#define SOME_DIR_NAME "some"
#define SOME_REQUIRED_LEN MAX(sizeof(SOME_FILE_NAME), sizeof(SOME_DIR_NAME))

static const char *disk_mount_pt = DISK_MOUNT_PT;


static int lsdir(const char *path)
{
	int res;
	struct fs_dir_t dirp;
	static struct fs_dirent entry;
	int count = 0;

	fs_dir_t_init(&dirp);

	/* Verify fs_opendir() */
	res = fs_opendir(&dirp, path);
	if (res) {
		printk("Error opening dir %s [%d]\n", path, res);
		return res;
	}

	printk("\nListing dir %s ...\n", path);
	for (;;) {
		/* Verify fs_readdir() */
		res = fs_readdir(&dirp, &entry);

		/* entry.name[0] == 0 means end-of-dir */
		if (res || entry.name[0] == 0) {
			break;
		}

		if (entry.type == FS_DIR_ENTRY_DIR) {
			printk("[DIR ] %s\n", entry.name);
		} else {
			printk("[FILE] %s (size = %zu)\n",
				entry.name, entry.size);
		}
		count++;
	}

	/* Verify fs_closedir() */
	fs_closedir(&dirp);
	if (res == 0) {
		res = count;
	}

	return res;
}

void test_sdcard()
{
	/* raw disk i/o */
	static const char *disk_pdrv = DISK_DRIVE_NAME;
	uint64_t memory_size_mb;
	uint32_t block_count;
	uint32_t block_size;

	if (disk_access_ioctl(disk_pdrv,
			DISK_IOCTL_CTRL_INIT, NULL) != 0) {
		LOG_ERR("Storage init ERROR!");
		return 0;
	}

	if (disk_access_ioctl(disk_pdrv,
			DISK_IOCTL_GET_SECTOR_COUNT, &block_count)) {
		LOG_ERR("Unable to get sector count");
		return 0;
	}
	LOG_INF("Block count %u", block_count);

	if (disk_access_ioctl(disk_pdrv,
			DISK_IOCTL_GET_SECTOR_SIZE, &block_size)) {
		LOG_ERR("Unable to get sector size");
		return 0;
	}
	printk("Sector size %u\n", block_size);

	memory_size_mb = (uint64_t)block_count * block_size;
	printk("Memory Size(MB) %u\n", (uint32_t)(memory_size_mb >> 20));

	if (disk_access_ioctl(disk_pdrv,
			DISK_IOCTL_CTRL_DEINIT, NULL) != 0) {
		LOG_ERR("Storage deinit ERROR!");
		return 0;
	}

	mp.mnt_point = disk_mount_pt;

	int res = fs_mount(&mp);

	if (res == FS_RET_OK) {
		printk("Disk mounted.\n");
		/* Try to unmount and remount the disk */
		res = fs_unmount(&mp);
		if (res != FS_RET_OK) {
			printk("Error unmounting disk\n");
			return res;
		}
		res = fs_mount(&mp);
		if (res != FS_RET_OK) {
			printk("Error remounting disk\n");
			return res;
		}

		if (lsdir(disk_mount_pt) == 0) {
#ifdef CONFIG_FS_SAMPLE_CREATE_SOME_ENTRIES
			if (create_some_entries(disk_mount_pt)) {
				lsdir(disk_mount_pt);
			}
#endif
		}
	} else {
		printk("Error mounting disk.\n");
	}

	fs_unmount(&mp);
}