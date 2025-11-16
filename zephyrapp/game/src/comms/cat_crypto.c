#include "cat_crypto.h"

#include "cat_math.h"
#include "cat_air.h"
#include "cat_persist.h"

uint32_t CAT_ZK_CO2()
{
	float co2 = CAT_clamp(readings.sunrise.ppm_filtered_uncompensated, 400, 4225);
	float t = CAT_inv_lerp(co2, 400, 4225);
	return CAT_clamp(t*255, 0, 255);
}

uint32_t CAT_ZK_PM2_5()
{
	return CAT_clamp(readings.sen5x.pm2_5, 0, 255);
}

uint32_t CAT_ZK_temp()
{
	float temp = CAT_clamp(CAT_canonical_temp(), -100, 100);
	float t = CAT_inv_lerp(temp, -100, 100);
	return CAT_clamp(t*255, 0, 255);
}

uint32_t CAT_ZK_stroop()
{
	if(stroop_data_valid)
	{
		float t = CAT_clamp(stroop_correctness, 0, 1);
		return CAT_clamp(t*255, 0, 255);
	}
	return 0;
}

uint32_t CAT_ZK_survey()
{
	return survey_mask;
}