#pragma once

#include "cat_core.h"

float CAT_co2_score(float co2);
float CAT_voc_score(float voc);
float CAT_nox_score(float nox);
float CAT_pm25_score(float pm25);
float CAT_canonical_temp();
float CAT_temperature_score(float temp);
float CAT_humidity_score(float rh);
float CAT_iaq_score(float co2, float voc, float nox, float pm25, float temp, float rh);
float CAT_aq_aggregate_score();

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

const char* CAT_get_AQM_unit(int aqm);

extern float CAT_AQ_raw_scores[CAT_AQM_COUNT];
void CAT_AQ_store_raw_scores();

extern float CAT_AQ_normalized_scores[CAT_AQM_COUNT];
void CAT_AQ_store_normalized_scores();

typedef enum
{
	CAT_AQ_SKULL_XP_UP
} CAT_AQ_skull;
