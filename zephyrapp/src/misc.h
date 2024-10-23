void turn_on_backlight();
void turn_on_3v3();
void test_speaker();
void init_pin(const struct gpio_dt_spec* pin, char* name, gpio_flags_t flags);
void init_matrix();
uint16_t scan_matrix();