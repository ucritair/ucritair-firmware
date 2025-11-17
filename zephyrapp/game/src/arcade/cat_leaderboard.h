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

extern uint8_t survey_field;