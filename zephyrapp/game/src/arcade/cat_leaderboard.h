#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct __attribute__((__packed__))
{
	float mean_time_cong;
	float mean_time_incong;
	uint8_t throughput;
} CAT_stroop_data;

extern int snake_highscore;
extern int mines_highscore;
extern int foursquares_highscore;

extern CAT_stroop_data stroop_data;
extern bool stroop_data_valid;
extern float stroop_correctness;

extern uint8_t survey_mask;

// RTC timestamp clamped from u64 to u32
uint32_t CAT_ZK_timestamp();
// PPM CO2 mapped from f32 to u32
uint32_t CAT_ZK_CO2();
// UG/M3 PM 2.5 mapped from f32 to u32
uint32_t CAT_ZK_PM2_5();
// Degrees celsius mapped from f32 to u32
uint32_t CAT_ZK_temp();
// % Correct mapped from f32 to u32
uint32_t CAT_ZK_stroop();
// 8 bits of survey information
uint32_t CAT_ZK_survey();