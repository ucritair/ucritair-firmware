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

static const char* CAT_AQM_titles[] =
{
	[CAT_AQM_CO2] = "CO2",
	[CAT_AQM_PM2_5] = "PM2.5",
	[CAT_AQM_NOX] = "NOX",
	[CAT_AQM_VOC] = "VOC",
	[CAT_AQM_TEMP] = "TEMP",
	[CAT_AQM_RH] = "RH",
	[CAT_AQM_AGGREGATE] = "\4CritAQ Score"
};

const char* CAT_get_AQM_unit_string(int aqm);

float CAT_AQ_get_raw_score(int aqm);
void CAT_AQ_store_raw_scores();

float CAT_AQ_get_normalized_score(int aqm);
void CAT_AQ_normalize_scores();

void CAT_AQ_tick();
