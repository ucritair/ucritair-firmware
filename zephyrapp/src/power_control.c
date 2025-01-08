
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/sys/poweroff.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(power_control, LOG_LEVEL_DBG);

#include "misc.h"
#include "power_control.h"
#include "rtc.h"
#include "buttons.h"
#include "lcd_rendering.h"

bool is_3v3_on, is_5v0_on, is_leds_on;

static const struct gpio_dt_spec pin_sen55_boost_enable =
	GPIO_DT_SPEC_GET(DT_NODELABEL(cat_misc), sen55_boost_enable_gpios);

static const struct gpio_dt_spec pin_buck_enable =
	GPIO_DT_SPEC_GET(DT_NODELABEL(cat_misc), buck_enable_gpios);

static const struct gpio_dt_spec pin_led_enable =
	GPIO_DT_SPEC_GET(DT_NODELABEL(cat_misc), led_enable_gpios);

void init_power_control()
{
	LOG_DBG("Init");
	init_pin(&pin_buck_enable, "pin_buck_enable", GPIO_OUTPUT_ACTIVE);
	init_pin(&pin_sen55_boost_enable, "pin_sen55_boost_enable", GPIO_OUTPUT_INACTIVE);
	init_pin(&pin_led_enable, "pin_led_enable", GPIO_OUTPUT_INACTIVE);

	// is_3v3_on = false;
	is_5v0_on = false;
	is_leds_on = false;
}

void set_3v3(bool on)
{
	// LOG_DBG("set_3v3(%d)", on);
	// pin_write(&pin_buck_enable, on);
	// k_msleep(10);
	// is_3v3_on = on;
}

void set_5v0(bool on)
{
	// LOG_DBG("set_5v0(%d)", on);
	pin_write(&pin_sen55_boost_enable, on);
	k_msleep(10);
	is_5v0_on = on;
}

void set_leds(bool on)
{
	// LOG_DBG("set_leds(%d)", on);
	pin_write(&pin_led_enable, on);
	k_msleep(10);
	is_leds_on = on;
}

#include <hal/nrf_rtc.h>
#include <hal/nrf_twim.h>
#include <hal/nrf_spim.h>
#include <hal/nrf_saadc.h>
#include <hal/nrf_timer.h>
#include <hal/nrf_usbd.h>
#include <hal/nrf_pwm.h>
#include <hal/nrf_clock.h>
#include <hal/nrf_egu.h>
#include <hal/nrf_oscillators.h>
#include <hal/nrf_wdt.h>
#include <nrfx_timer.h>
#include <soc/nrfx_coredep.h>

static struct gpio_callback button_cb_data;
static const struct device* gpio0 = DEVICE_DT_GET(DT_NODELABEL(gpio0));
static const struct device* gpio1 = DEVICE_DT_GET(DT_NODELABEL(gpio1));

volatile enum {
	WAKE_CAUSE_NONE,
	WAKE_CAUSE_TIMER,
	WAKE_CAUSE_BUTTON
} wake_cause = WAKE_CAUSE_NONE;

void configure_buttons_for_sleep()
{
	for (int i = 9; i <= 12; i++)
	{
		gpio_pin_configure(gpio1, i, GPIO_INPUT | GPIO_PULL_UP);
		gpio_pin_interrupt_configure(gpio1, i, GPIO_INT_EDGE_FALLING);
	}

	gpio_pin_configure(gpio1, 13, GPIO_OUTPUT_LOW);
	gpio_pin_configure(gpio1, 14, GPIO_OUTPUT_LOW);
}

void button_pressed(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	wake_cause = WAKE_CAUSE_BUTTON;
}

void on_rtc_compare3()
{
    wake_cause = WAKE_CAUSE_TIMER;
}

#include <hal/nrf_gpio.h>

void power_off(int for_ms, bool protected_sleeping)
{
	LOG_INF("power_off(%d)", for_ms);

	if (cat_game_running)
	{
		went_to_sleep_at = get_current_rtc_time();
		LOG_INF("Saving game...");
		CAT_force_save();
	}
	// fails if active
	// struct net_if *iface = net_if_get_default();
	// net_if_down(iface);

	if (bt_is_ready())
	{
		bt_le_adv_stop();
		bt_disable();
	}

	set_5v0(false);
	set_leds(false);

	// Drop IOVDD_EN
	if (gpio_pin_configure(gpio0, 11, GPIO_OUTPUT_HIGH)) LOG_ERR("Init 0.11 failed");
	gpio_pin_set(gpio0, 11, true);

	// Drop BUCKEN
	if (gpio_pin_configure(gpio0, 21, GPIO_OUTPUT_LOW)) LOG_ERR("Init 0.21 failed");
	gpio_pin_set(gpio0, 21, false);

	
	nrf_spim_disable(NRF_SPIM0);
	nrf_spim_disable(NRF_SPIM1);
	nrf_spim_disable(NRF_SPIM2);
	nrf_spim_disable(NRF_SPIM3);
	nrf_spim_disable(NRF_SPIM4);

	nrf_twim_disable(NRF_TWIM0);
	nrf_twim_disable(NRF_TWIM1);
	nrf_twim_disable(NRF_TWIM2);
	nrf_twim_disable(NRF_TWIM3);

	nrf_saadc_disable(NRF_SAADC);

	nrf_timer_mode_set(NRF_TIMER0, NRF_TIMER_MODE_LOW_POWER_COUNTER);
	nrf_timer_mode_set(NRF_TIMER1, NRF_TIMER_MODE_LOW_POWER_COUNTER);
	nrf_timer_mode_set(NRF_TIMER2, NRF_TIMER_MODE_LOW_POWER_COUNTER);

	uint32_t mask = NRF_TIMER_INT_COMPARE0_MASK|NRF_TIMER_INT_COMPARE1_MASK|
					NRF_TIMER_INT_COMPARE2_MASK|NRF_TIMER_INT_COMPARE3_MASK|
					NRF_TIMER_INT_COMPARE4_MASK|NRF_TIMER_INT_COMPARE5_MASK;
	nrf_timer_int_disable(NRF_TIMER0, mask);
	nrf_timer_int_disable(NRF_TIMER1, mask);
	nrf_timer_int_disable(NRF_TIMER2, mask);

	nrf_pwm_disable(NRF_PWM0);
	nrf_pwm_disable(NRF_PWM1);
	nrf_pwm_disable(NRF_PWM2);
	nrf_pwm_disable(NRF_PWM3);

	// ============ IS THIS CRITICAL FOR LOW POWER SLEEP?? ===============
	nrf_usbd_disable(NRF_USBD);

	nrf_clock_alwaysrun_set(NRF_CLOCK, NRF_CLOCK_DOMAIN_HFCLK, false);
	nrf_clock_alwaysrun_set(NRF_CLOCK, NRF_CLOCK_DOMAIN_HFCLK192M, false);
	nrf_clock_alwaysrun_set(NRF_CLOCK, NRF_CLOCK_DOMAIN_HFCLKAUDIO, false);

	nrf_egu_int_disable(NRF_EGU0, NRF_EGU_INT_ALL);
	nrf_egu_int_disable(NRF_EGU1, NRF_EGU_INT_ALL);
	nrf_egu_int_disable(NRF_EGU2, NRF_EGU_INT_ALL);
	nrf_egu_int_disable(NRF_EGU3, NRF_EGU_INT_ALL);
	nrf_egu_int_disable(NRF_EGU4, NRF_EGU_INT_ALL);
	nrf_egu_int_disable(NRF_EGU5, NRF_EGU_INT_ALL);

	pin_write(&pin_buck_enable, false);


	for (int i = 2; i < 32; i++)
	{
		gpio_pin_configure(gpio0, i, GPIO_DISCONNECTED);
	}

	for (int i = 0; i < 16; i++)
	{
		gpio_pin_configure(gpio1, i, GPIO_DISCONNECTED);
	}

	// set_5v0(false);
	// set_leds(false);

	nrfx_coredep_delay_us(1000*10);

	if (!protected_sleeping)
	{
		configure_buttons_for_sleep();

		gpio_init_callback(&button_cb_data, button_pressed, BIT(9)|BIT(10)|BIT(11)|BIT(12));
		gpio_add_callback(gpio1, &button_cb_data);
	}

	NRF_CLOCK_S->HFCLKCTRL = (CLOCK_HFCLKCTRL_HCLK_Div2 << CLOCK_HFCLKCTRL_HCLK_Pos); // 64MHz

	*(volatile uint32_t*)(0x50005000 + 0x614) = 1; // Force network core off

	if (for_ms)
	{
		configure_rtc_timer3(for_ms);
	}

	while (1)
	{
		wake_cause = WAKE_CAUSE_NONE;
		__asm("wfi");

		if (wake_cause == WAKE_CAUSE_NONE)
			continue;

		if (wake_cause == WAKE_CAUSE_TIMER)
		{
			wakeup_is_from_timer = true;
		}
		else
		{
			LOG_INF("WAKE_CAUSE_BUTTON");
			wakeup_is_from_timer = false;
			// WAKE_CAUSE_BUTTON

			nrfx_coredep_delay_us(1000*5);

			// init_buttons();

			bool decided_to_wake = true;

			for (int cycle = 0; cycle < 10; cycle++)
			{
				nrfx_coredep_delay_us(1000*1);
				// update_buttons();

				uint32_t bits;
				int err = (gpio_port_get_raw(gpio1, &bits));

				bits >>= 9;
				bits &= 0b1111;

				LOG_DBG("wakeup debounce = %x", bits);

				if (err)
				{
					LOG_ERR("Failed to gpio_port_get_raw");
				}

				if (bits == 0b1111)
				{
					LOG_INF("Button wake debounce failed, bits=%x", bits);
					decided_to_wake = false;
					break;
				}
			}

			if (!decided_to_wake)
			{
				LOG_INF("Decided to go back to sleep");
				// k_msleep(100);

				configure_buttons_for_sleep();
				continue;
			}
		}

		LOG_INF("Waking...");
		// k_msleep(100);

		snapshot_rtc_for_reboot();
		nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(0, 2));
		sys_reboot(SYS_REBOOT_WARM);
	}
}

#include <zephyr/init.h>
#include <hal/nrf_power.h>

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

     nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(1, 4)); // SEN55_BOOST
     nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(1, 4)); // SEN55_BOOST
     nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(0, 5)); // BUCK_ENABLE
     nrf_gpio_pin_set(NRF_GPIO_PIN_MAP(0, 5)); // BUCK_ENABLE
     nrfx_coredep_delay_us(1000*10);

    return 0;
}

SYS_INIT(board_cat_uicr_init, PRE_KERNEL_1,
     CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);


bool get_is_charging()
{
	return NRF_USBREGULATOR->USBREGSTATUS & 1;
}