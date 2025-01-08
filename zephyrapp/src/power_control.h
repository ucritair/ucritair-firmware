
void init_power_control();

extern bool is_3v3_on, is_5v0_on, is_leds_on;

void set_3v3(bool on);
void set_5v0(bool on);
void set_leds(bool on);
void power_off(int for_ms, bool protected);
void on_rtc_compare3();

bool get_is_charging();