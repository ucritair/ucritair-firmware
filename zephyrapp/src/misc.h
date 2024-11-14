#include <zephyr/drivers/gpio.h>

void init_pin(const struct gpio_dt_spec* pin, char* name, gpio_flags_t flags);
void pin_write(const struct gpio_dt_spec* pin, bool v);
void test_i2c();
void report_ns2009();

#define SYS_FW_VERSION "sea-1"
