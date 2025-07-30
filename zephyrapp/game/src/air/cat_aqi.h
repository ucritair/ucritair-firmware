#pragma once

#include "cat_core.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// SCORING

float CAT_CO2_score(float co2);
float CAT_VOC_score(float voc);
float CAT_NOX_score(float nox);
float CAT_PM2_5_score(float pm25);
float CAT_canonical_temp();
float CAT_wet_bulb_temp(float air_degc);
float CAT_temp_score(float temp);
float CAT_rh_score(float rh);
float CAT_IAQ_score(float co2, float voc, float nox, float pm25, float temp, float rh);
float CAT_AQ_aggregate_score();

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

const char* CAT_AQ_get_title_string(int aqm);
const char* CAT_AQ_get_unit_string(int aqm);
const char* CAT_AQ_get_grade_string(float score);
uint16_t CAT_AQ_get_grade_colour(float score);

void CAT_AQ_store_live_scores();
float CAT_AQ_live_score_raw(int aqm);
float CAT_AQ_live_score_normalized(int aqm);

void CAT_AQ_tick();

float CAT_AQ_block_score_raw(CAT_AQ_score_block* block, int aqm);
float CAT_AQ_block_score_normalized(CAT_AQ_score_block* block, int aqm);
