/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 * Copyright (c) 2020 Arm Limited
 * Copyright (c) 2021-2023 Nordic Semiconductor ASA
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <assert.h>
#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/drivers/timer/system_timer.h>
#include <zephyr/usb/usb_device.h>
#include <soc.h>
#include <zephyr/linker/linker-defs.h>

#include "target.h"
#include "bootutil/bootutil_log.h"

#if defined(CONFIG_BOOT_SERIAL_PIN_RESET) || defined(CONFIG_BOOT_FIRMWARE_LOADER_PIN_RESET)
#include <zephyr/drivers/hwinfo.h>
#endif

#if defined(CONFIG_BOOT_SERIAL_BOOT_MODE) || defined(CONFIG_BOOT_FIRMWARE_LOADER_BOOT_MODE)
#include <zephyr/retention/bootmode.h>
#endif

/* Validate serial recovery configuration */
#ifdef CONFIG_MCUBOOT_SERIAL
#if !defined(CONFIG_BOOT_SERIAL_ENTRANCE_GPIO) && \
    !defined(CONFIG_BOOT_SERIAL_WAIT_FOR_DFU) && \
    !defined(CONFIG_BOOT_SERIAL_BOOT_MODE) && \
    !defined(CONFIG_BOOT_SERIAL_NO_APPLICATION) && \
    !defined(CONFIG_BOOT_SERIAL_PIN_RESET)
#error "Serial recovery selected without an entrance mode set"
#endif
#endif

/* Validate firmware loader configuration */
#ifdef CONFIG_BOOT_FIRMWARE_LOADER
#if !defined(CONFIG_BOOT_FIRMWARE_LOADER_ENTRANCE_GPIO) && \
    !defined(CONFIG_BOOT_FIRMWARE_LOADER_BOOT_MODE) && \
    !defined(CONFIG_BOOT_FIRMWARE_LOADER_NO_APPLICATION) && \
    !defined(CONFIG_BOOT_FIRMWARE_LOADER_PIN_RESET)
#error "Firmware loader selected without an entrance mode set"
#endif
#endif

#ifdef CONFIG_MCUBOOT_INDICATION_LED

/*
 * The led0 devicetree alias is optional. If present, we'll use it
 * to turn on the LED whenever the button is pressed.
 */
#if DT_NODE_EXISTS(DT_ALIAS(mcuboot_led0))
#define LED0_NODE DT_ALIAS(mcuboot_led0)
#elif DT_NODE_EXISTS(DT_ALIAS(bootloader_led0))
#warning "bootloader-led0 alias is deprecated; use mcuboot-led0 instead"
#define LED0_NODE DT_ALIAS(bootloader_led0)
#endif

#if DT_NODE_HAS_STATUS(LED0_NODE, okay) && DT_NODE_HAS_PROP(LED0_NODE, gpios)
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
#else
/* A build error here means your board isn't set up to drive an LED. */
#error "Unsupported board: led0 devicetree alias is not defined"
#endif

BOOT_LOG_MODULE_DECLARE(mcuboot);

void io_led_init(void)
{
    if (!device_is_ready(led0.port)) {
        BOOT_LOG_ERR("Didn't find LED device referred by the LED0_NODE\n");
        return;
    }

    gpio_pin_configure_dt(&led0, GPIO_OUTPUT);
    gpio_pin_set_dt(&led0, 0);
}

void io_led_set(int value)
{
    gpio_pin_set_dt(&led0, value);
}
#endif /* CONFIG_MCUBOOT_INDICATION_LED */

#if defined(CONFIG_BOOT_SERIAL_ENTRANCE_GPIO) || defined(CONFIG_BOOT_USB_DFU_GPIO) || \
    defined(CONFIG_BOOT_FIRMWARE_LOADER_ENTRANCE_GPIO)

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <hal/nrf_power.h>

#define DFU_MAGIC_VALUE 0xB1

#define N_ROWS 4
static const struct gpio_dt_spec btn_rows[N_ROWS] =
{
    {.port = DEVICE_DT_GET(DT_NODELABEL(gpio1)), .pin=9, .dt_flags=GPIO_ACTIVE_HIGH},
    {.port = DEVICE_DT_GET(DT_NODELABEL(gpio1)), .pin=10, .dt_flags=GPIO_ACTIVE_HIGH},
    {.port = DEVICE_DT_GET(DT_NODELABEL(gpio1)), .pin=11, .dt_flags=GPIO_ACTIVE_HIGH},
    {.port = DEVICE_DT_GET(DT_NODELABEL(gpio1)), .pin=12, .dt_flags=GPIO_ACTIVE_HIGH},
};

#define N_COLS 2
static const struct gpio_dt_spec btn_cols[N_COLS] =
{
    {.port = DEVICE_DT_GET(DT_NODELABEL(gpio1)), .pin=13, .dt_flags=GPIO_ACTIVE_HIGH},
    {.port = DEVICE_DT_GET(DT_NODELABEL(gpio1)), .pin=14, .dt_flags=GPIO_ACTIVE_HIGH},
};

static const struct gpio_dt_spec reg_3v3_dt = {.port = DEVICE_DT_GET(DT_NODELABEL(gpio0)), .pin=5, .dt_flags=GPIO_ACTIVE_HIGH};
static const struct gpio_dt_spec en_7002 = {.port = DEVICE_DT_GET(DT_NODELABEL(gpio0)), .pin=11, .dt_flags=GPIO_ACTIVE_HIGH};
static const struct gpio_dt_spec buck_7002 = {.port = DEVICE_DT_GET(DT_NODELABEL(gpio0)), .pin=21, .dt_flags=GPIO_ACTIVE_HIGH};
static const struct gpio_dt_spec reg_5v = {.port = DEVICE_DT_GET(DT_NODELABEL(gpio1)), .pin=4, .dt_flags=GPIO_ACTIVE_HIGH};
static const struct gpio_dt_spec epd_reset = {.port = DEVICE_DT_GET(DT_NODELABEL(gpio1)), .pin=5, .dt_flags=GPIO_ACTIVE_HIGH};

void init_buttons()
{
    // gpio_pin_configure_dt(&en_7002, GPIO_OUTPUT_HIGH);
    gpio_pin_configure_dt(&buck_7002, GPIO_OUTPUT_LOW);

    gpio_pin_configure_dt(&reg_5v, GPIO_OUTPUT_LOW);
    // gpio_pin_configure_dt(&epd_reset, GPIO_OUTPUT_LOW);

    // gpio_pin_configure_dt(&reg_3v3_dt, GPIO_OUTPUT_HIGH);

    for (int i = 0; i < N_ROWS; i++)
    {
        gpio_pin_configure_dt(&btn_rows[i], GPIO_INPUT|GPIO_PULL_UP);
    }

    for (int i = 0; i < N_COLS; i++)
    {
        gpio_pin_configure_dt(&btn_cols[i], GPIO_OUTPUT_ACTIVE);
        gpio_pin_set_dt(&btn_cols[i], true);
    }
}

uint8_t get_buttons()
{
    uint8_t bits = 0;

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

    return ~bits;
}

#define CAT_BTN_MASK_SELECT 0x01
#define CAT_BTN_MASK_START 0x02
#define CAT_BTN_MASK_A 0x04
#define CAT_BTN_MASK_B 0x08
#define CAT_BTN_MASK_DOWN 0x10
#define CAT_BTN_MASK_RIGHT 0x20
#define CAT_BTN_MASK_LEFT 0x40
#define CAT_BTN_MASK_UP 0x80

const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

static const struct gpio_dt_spec lcd_bl = {.port = DEVICE_DT_GET(DT_NODELABEL(gpio1)), .pin=0, .dt_flags=GPIO_ACTIVE_HIGH};

#define LCD_IMAGE_H 320
#define LCD_IMAGE_W 240

#include <zephyr/drivers/display.h>

uint16_t lcd_framebuffer[LCD_IMAGE_H*LCD_IMAGE_W] = {0};

#include "font_8x8_basic.cpp"

void lcd_write_char(uint16_t color, int x, int y, char c)
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
                        int fy = (((y+by)*scale)+sy);
                        int fx = ((x+bx)*scale)+sx;
                        
                        if (fx < 0 || fx >= LCD_IMAGE_W) continue;
                        if (fy < 0 || fy >= LCD_IMAGE_H) continue;

                        lcd_framebuffer[(fy * LCD_IMAGE_W) + fx] = color;
                    }
                }
            }
        }
    }
}

void lcd_write_str(uint16_t color, int x, int y, char* str)
{
    int ox = x;
    while (*str)
    {
        lcd_write_char(color, x, y, *str);
        x += 8;
        if ((*str) == '\n')
        {
            x = ox;
            y += 8;
        }
        str++;
    }
}

bool io_detect_pin()
{
    // Check if application requested DFU via GPREGRET retention register
    uint8_t gpregret = nrf_power_gpregret_get(NRF_POWER, 0);
    if (gpregret == DFU_MAGIC_VALUE)
    {
        nrf_power_gpregret_set(NRF_POWER, 0, 0);
        return true;
    }

    // Check physical button combo: SELECT+START+DOWN
    init_buttons();

    k_msleep(25);

    for (int i = 0; i < 10; i++)
    {
        uint8_t b = get_buttons();
        if (!(b == (CAT_BTN_MASK_SELECT|CAT_BTN_MASK_START|CAT_BTN_MASK_DOWN)))
            return false;

        k_msleep(5);
    }

    return true;
}

void indicate_waiting()
{
    gpio_pin_configure_dt(&lcd_bl, GPIO_OUTPUT_ACTIVE);
    gpio_pin_set_dt(&lcd_bl, true);

    struct display_buffer_descriptor desc = {
        .buf_size = LCD_IMAGE_H*LCD_IMAGE_W,
        .width = LCD_IMAGE_W,
        .height = LCD_IMAGE_H,
        .pitch = LCD_IMAGE_W
    };

    int y = 0;

    lcd_write_str(0xffff, 0, (y+=8), "~~CAT BOOTLOADER MODE~~");
    lcd_write_str(0xffff, 0, (y+=8), "Connect to a computer over USB");
    lcd_write_str(0xffff, 0, (y+=8), "to download new firmware.");

    y+=8;

    lcd_write_str(0xffff, 0, (y+=8), "Or press RESET without holding");
    lcd_write_str(0xffff, 0, (y+=8), "other keys to boot normally.");

    y+=8;

    lcd_write_str(0xffff, 0, (y+=8), "This is hw rev 0.2.");
    lcd_write_str(0xffff, 0, (y+=8), "This is bl rev 3 (einit).");

    y+=128+32+32-8;

    lcd_write_str(0x0ff0, 0, (y+=8), "  _.._");
    lcd_write_str(0x0ff0, 0, (y+=8), ".' .-'`");
    lcd_write_str(0x0ff0, 0, (y+=8), "/  /  search and you will find");
    lcd_write_str(0x0ff0, 0, (y+=8), "|  |     an end is a beginning");
    lcd_write_str(0x0ff0, 0, (y+=8), "\\  \\    the harvest moon wanes");
    lcd_write_str(0x0ff0, 0, (y+=8), "'._'-._");
    lcd_write_str(0x0ff0, 0, (y+=8), "   ```");
    
    display_write(display_dev, 0, 0, &desc, lcd_framebuffer);

    // lcd_flip();

    display_blanking_off(display_dev);
}

#include <zephyr/init.h>
#include <hal/nrf_power.h>
#include <hal/nrf_gpio.h>
#include <soc/nrfx_coredep.h>

static int board_cat_uicr_init(void)
{
    if (NRF_UICR->VREGHVOUT != 0x5) {

        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {
            ;
        }

        NRF_UICR->VREGHVOUT = 0x5;

        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {
            ;
        }

        /* a reset is required for changes to take effect */
        NVIC_SystemReset();
    }

    // while (1) {}

    nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(1, 4)); // SEN55_BOOST
    nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(1, 4)); // SEN55_BOOST

    nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(0, 5)); // BUCK_ENABLE
    nrf_gpio_pin_set(NRF_GPIO_PIN_MAP(0, 5)); // BUCK_ENABLE
    nrfx_coredep_delay_us(1000*10);

    // while (1) {}

    return 0;
}

SYS_INIT(board_cat_uicr_init, PRE_KERNEL_1,
     CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);



#endif

#if defined(CONFIG_BOOT_SERIAL_PIN_RESET) || defined(CONFIG_BOOT_FIRMWARE_LOADER_PIN_RESET)
bool io_detect_pin_reset(void)
{
    uint32_t reset_cause;
    int rc;

    rc = hwinfo_get_reset_cause(&reset_cause);

    if (rc == 0 && reset_cause == RESET_PIN) {
        (void)hwinfo_clear_reset_cause();
        return true;
    }

    return false;
}
#endif

#if defined(CONFIG_BOOT_SERIAL_BOOT_MODE) || defined(CONFIG_BOOT_FIRMWARE_LOADER_BOOT_MODE)
bool io_detect_boot_mode(void)
{
    int32_t boot_mode;

    boot_mode = bootmode_check(BOOT_MODE_TYPE_BOOTLOADER);

    if (boot_mode == 1) {
        /* Boot mode to stay in bootloader, clear status and enter serial
         * recovery mode
         */
        bootmode_clear();

        return true;
    }

    return false;
}
#endif
