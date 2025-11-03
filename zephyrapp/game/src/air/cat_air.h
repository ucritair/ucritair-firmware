#pragma once

#include "cat_core.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND TYPES

#define CAT_AQ_MOVING_SAMPLE_PERIOD 5
#define CAT_AQ_SPARKLINE_SAMPLE_PERIOD CAT_DAY_SECONDS

typedef enum
{
	CAT_AQM_CO2,
	CAT_AQM_PM2_5,
	CAT_AQM_NOX,
	CAT_AQM_VOC,
	CAT_AQM_TEMP,
	CAT_AQM_RH,
	CAT_AQM_AGGREGATE,
	CAT_AQM_COUNT,
} CAT_AQM;

typedef enum
{
	CAT_TEMPERATURE_UNIT_DEGREES_CELSIUS,
	CAT_TEMPERATURE_UNIT_DEGREES_FAHRENHEIT
} CAT_temperature_unit;

typedef struct __attribute__((__packed__))
{
	uint16_t CO2; // ppm x1
	uint8_t VOC; // index x1
	uint8_t NOX; // index x1
	uint16_t PM2_5; // ug/m3 x100
	int32_t temp; // degC x1000
	uint16_t rh; // % x100
	uint8_t aggregate; // score x1
} CAT_AQ_score_block;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TEMPERATURE

CAT_temperature_unit CAT_AQ_get_temperature_unit();
void CAT_AQ_set_temperature_unit(CAT_temperature_unit unit);
float CAT_AQ_map_celsius(float temp);
const char* CAT_AQ_get_temperature_unit_string();

float CAT_canonical_temp();
float CAT_wet_bulb_temp(float air_degc);


////////////////////////////////////////////////////////////////////////////////////////////////////
// SCORING

float CAT_CO2_score(float co2);
float CAT_VOC_score(float voc);
float CAT_NOX_score(float nox);
float CAT_PM2_5_score(float pm25);
float CAT_temp_score(float temp);
float CAT_RH_score(float rh);
float CAT_IAQ_score(float co2, float voc, float nox, float pm25, float temp, float rh);
float CAT_AQ_aggregate_score();


////////////////////////////////////////////////////////////////////////////////////////////////////
// AQM INFO

const char* CAT_AQ_title_string(int aqm);
const char* CAT_AQ_unit_string(int aqm);
const char* CAT_AQ_grade_string(float score);
uint16_t CAT_AQ_grade_colour(float score);


////////////////////////////////////////////////////////////////////////////////////////////////////
// READINGS

void CAT_AQ_poll_readings();
float CAT_AQ_value(int aqm);
float CAT_AQ_score(int aqm);
float CAT_AQ_delta(int aqm);

float CAT_AQ_block_value(CAT_AQ_score_block* block, int aqm);
float CAT_AQ_block_score(CAT_AQ_score_block* block, int aqm);

bool CAT_AQ_moving_scores_initialized();
void CAT_AQ_update_moving_scores();

bool CAT_AQ_weekly_scores_initialized();
void CAT_AQ_push_weekly_scores(CAT_AQ_score_block* in);
CAT_AQ_score_block* CAT_AQ_get_weekly_scores(int idx);

void CAT_AQ_tick();
