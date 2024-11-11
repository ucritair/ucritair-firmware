#include "cat_core.h"
#include "cat_math.h"

float CAT_temp_score()
{
	float mean_temp = 0;
	int mean_count = 0;
	if (aqi.lps22hh.uptime_last_updated)
	{
		mean_temp += aqi.lps22hh.temp;
		mean_count++;
	}

	if (aqi.sunrise.uptime_last_updated)
	{
		mean_temp += aqi.sunrise.temp;
		mean_count++;
	}

	if (aqi.sen5x.uptime_last_updated)
	{
		mean_temp += aqi.sen5x.temp_degC;
		mean_count++;
	}

	mean_temp /= mean_count;

	if (mean_count == 0)
	{
		// none ready
		return 0.5;
	}

	return inv_lerp(mean_temp, 15, 24);
}

float CAT_CO2_score()
{
	if (aqi.sunrise.uptime_last_updated == 0)
	{
		// not ready yet
		return 0.5;
	}

	return inv_lerp(aqi.sunrise.ppm_filtered_compensated, 250, 1000);
}

float CAT_PM_score()
{
	if (aqi.sen5x.uptime_last_updated == 0)
	{
		// not ready yet
		return 0.5;
	}

	float pm_sm = aqi.sen5x.pm2_5;
	float pm_lg = aqi.sen5x.pm10_0 - pm_sm;
	float sm_score = inv_lerp(pm_sm, 0, 15);
	float lg_score = inv_lerp(pm_lg, 0, 12);
	return (lg_score + 2 * sm_score) / 3;
}

float CAT_VOC_score()
{
	if (aqi.sen5x.voc_index == 0)
	{
		// not ready yet
		return 0.5;
	}

	return inv_lerp(aqi.sen5x.voc_index, 0.9, 100);
}

float CAT_NOX_score()
{
	if (aqi.sen5x.nox_index == 0)
	{
		// not ready yet
		return 0.5;
	}

	return inv_lerp(aqi.sen5x.nox_index, 90, 400);
}

void CAT_calc_quantized_aqi_scores(
	int* temp_idx,
	int* co2_idx,
	int* pm_idx,
	int* voc_idx,
	int* nox_idx)
{
	*temp_idx = quantize(CAT_temp_score(), 1, 3);
	*co2_idx = quantize(CAT_CO2_score(), 1, 3);
	*pm_idx = quantize(CAT_PM_score(), 1, 3);
	*voc_idx = quantize(CAT_VOC_score(), 1, 3);
	*nox_idx = quantize(CAT_NOX_score(), 1, 3);
}
