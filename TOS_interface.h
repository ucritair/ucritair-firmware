

////////////////////////////////////////////////////////////////////////////////////////////////////
// LCD SCREEN

#define LCD_SCREEN_W 240
#define LCD_SCREEN_H 320

// Argument `buffer` is a pointer to an array of LCD_SCREEN_W*LCD_SCREEN_H 16-bit pixel samples
// Each pixel sample is RGB565
// Calling this *starts* a display flip and (may) return immediately
void TOS_post_lcd_screen_buffer(uint16_t* buffer);

// Returns true if the last started display flip is now completed
bool TOS_is_lcd_screen_buffer_posted();

// Alternatively, we could not have a full framebuffer and do it in smaller updates:
void TOS_blit_lcd_screen_block(int x, int y, int w, int h, uint16_t* buffer);

void TOS_set_backlight(int percent);


////////////////////////////////////////////////////////////////////////////////////////////////////
// EINK SCREEN

#define EINK_SCREEN_W 212
#define EINK_SCREEN_H 104

// Argument `buffer` is a pointer to an array of (EINK_SCREEN_W*EINK_SCREEN_H)/8
//   8-bit packed pixel samples
// Each byte is eight consecutive pixels where black == 1
// Calling this *starts* a display flip and (may) return immediately
void TOS_post_eink_screen_buffer(uint8_t* buffer);

// Returns true if the last started display flip is now completed
bool TOS_is_eink_screen_buffer_posted();



////////////////////////////////////////////////////////////////////////////////////////////////////
// LEDs

#define NUM_LEDS 8

// Takes a pointer to an array of NUM_LEDS uint32_ts which are packed RGBX8888
void TOS_write_LED_states(uint32_t* leds);



////////////////////////////////////////////////////////////////////////////////////////////////////
// Sound

// TBD. At minimum will have...
void TOS_sound_play_tone(int hz, int duration_ms);



////////////////////////////////////////////////////////////////////////////////////////////////////
// Buttons

enum TOS_button
{
	TOS_button_A = 1 << 0,
	TOS_button_B = 1 << 1,
	TOS_button_START = 1 << 2,
	TOS_button_SELECT = 1 << 3,
	TOS_button_UP = 1 << 4,
	TOS_button_LEFT = 1 << 5,
	TOS_button_RIGHT = 1 << 6,
	TOS_button_DOWN = 1 << 7
};

// Returns bitmask of pressed buttons where pressed == 1
uint16_t TOS_get_button_states();

// x_value and y_value are connected to actual X and Y touch locations via a arbitrary and
//  per-device linear equation. Ideally this is such that 0 is one edge and (1<<16)-1 is the other
//  edge but in reality this will likely require calibration to work well.
struct TOS_touch_state
{
	uint16_t x_value;
	uint16_t y_value;
	uint16_t pressure_value;
};

void TOS_get_touchscreen_state(struct TOS_touch_state* state_out);



////////////////////////////////////////////////////////////////////////////////////////////////////
// Timing

// Return precise arbitrarily increasing monotonic time in microseconds
uint64_t TOS_get_time_ms();

struct TOS_realtime
{
	int year, month, day, hour, minute, second;
}

void TOS_get_realtime(struct TOS_realtime* time_out);
void TOS_set_realtime(struct TOS_realtime* time_in);



////////////////////////////////////////////////////////////////////////////////////////////////////
// Memory Management

// You can have malloc()/free() if you promise to be a good boy but really would prefer not
void* TOS_malloc(int bytes);
void TOS_free(void* ptr);

// We will likely need paging for assets which would look something like this:

// Exact size TBD
#define PAGING_PAGE_SIZE 4096*8

// How many pages of data we can have present in memory at the same time. Exact count TBD
#define PAGING_REGIONS 4

// Return how many paging regions are currently free
int TOS_get_free_paging_regions();

enum TOS_page
{
	TOS_page_HOUSE_BACKGROUNDS,
	TOS_page_HOUSE_PROPS,
	TOS_page_UI_VENDING,
	TOS_page_UI_SMOKING
	// ...
};

// Assuming there is an unoccupied paging slot, load the data from external flash and return a
//  pointer to it, otherwise return NULL
void* TOS_page_in(enum TOS_page page_id);

// Free a paging region, making references into it invalid
void TOS_page_out(void* page);

// And then we would have some scheme to implement packing things into paging regions TBD.
// The whole paging thing is moot if the memory footprint is small enough but I doubt it will be



////////////////////////////////////////////////////////////////////////////////////////////////////
// Persistence

#define PERSISTANCE_BLOCK_SIZE 4096 * 4
// Exact size TBD

void TOS_write_persistance_data(uint8_t* data_in);
bool TOS_check_persistance_data_exists();
void TOS_read_persistance_data(uint8_t* data_out);



////////////////////////////////////////////////////////////////////////////////////////////////////
// System sensors

// I don't remember the scaling on this
struct TOS_accel_data
{
	uint16_t x, y, z;
}

void TOS_get_accelerometer(struct TOS_accel_data* state_out);

// true if being charged
bool TOS_get_is_usb_connected();

int TOS_get_battery_level_pct();



////////////////////////////////////////////////////////////////////////////////////////////////////
// Wireless

// TBD



////////////////////////////////////////////////////////////////////////////////////////////////////
// Air Quality Sensors

// TBD, sketch:

int TOS_get_ms_since_last_aqi_reading();

struct sen55_data_sample
{
	uint16_t mass_pm1_0,
	         mass_pm2_5,
	         mass_pm4_0,
	         mass_pm10;

	int16_t rh,
	        temp,
	        voc,
	        nox;

	uint16_t count_pm0_5,
			 count_pm1_0,
	         count_pm2_5,
	         count_pm4_0,
	         count_pm10,
	         typ_size;
};

struct sunrise_data_sample
{
	uint16_t co2, temp;
};

struct lps22hh_data_sample
{
	uint16_t temp;
	uint32_t press;
};

void TOS_get_last_aqi_reading(
	struct sen55_data_sample* sen55,
	struct sunrise_data_sample* sunrise,
	struct lps22hh_data_sample* lps22hh);



////////////////////////////////////////////////////////////////////////////////////////////////////
// Power management

// TBD, sketch:

void TOS_power_off();
void TOS_sleep(int wakeup_after_seconds);