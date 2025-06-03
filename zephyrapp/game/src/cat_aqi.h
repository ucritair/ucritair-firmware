#pragma once

#include "cat_core.h"

float CAT_co2_score(float co2);
float CAT_voc_score(float voc);
float CAT_nox_score(float nox);
float CAT_pm25_score(float pm25);
float CAT_mean_temp();
float CAT_temperature_score(float temp);
float CAT_humidity_score(float rh);
float CAT_iaq_score(float co2, float voc, float nox, float pm25, float temp, float rh);

void CAT_AQI_dump_scores(float* scores);
void CAT_AQI_quantize(int* temp_idx, int* co2_idx, int* pm_idx, int* voc_idx, int* nox_idx);
float CAT_AQI_aggregate();