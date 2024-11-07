#include "cat_core.h"
#include "cat_math.h"

float CAT_temp_score()
{
	float mean_temp = (aqi.lps22hh.temp + aqi.sunrise.temp + aqi.sen5x.temp_degC) / 3.0f;
	return inv_lerp(mean_temp, 15, 24);
}

float CAT_CO2_score()
{
	return inv_lerp(aqi.sunrise.ppm_filtered_compensated, 250, 1000);
}

float CAT_PM_score()
{
	float pm_sm = aqi.sen5x.pm2_5;
	float pm_lg = aqi.sen5x.pm10_0 - pm_sm;
	float sm_score = inv_lerp(pm_sm, 0, 15);
	float lg_score = inv_lerp(pm_lg, 0, 12);
	return (lg_score + 2 * sm_score) / 3;
}

float CAT_VOC_score()
{
	return inv_lerp(aqi.sen5x.voc_index, 0.9, 100);
}

float CAT_NOX_score()
{
	return inv_lerp(aqi.sen5x.nox_index, 90, 400);
}