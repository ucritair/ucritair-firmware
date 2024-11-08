#include <time.h>

extern bool is_first_init;
extern uint64_t rtc_offset;
void zero_rtc_counter();
void check_rtc_init();
void snapshot_rtc_for_reboot();
time_t get_current_rtc_time();
void update_rtc();
void set_rtc_counter(struct tm* t);

#define PERSIST_RAM __attribute__((__section__(".endmap_presist_region")))

extern char* month_names[12];