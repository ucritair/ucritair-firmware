#include "cat_crypto.h"

#include "cat_math.h"
#include "cat_air.h"
#include "cat_persist.h"

#define ROUND_CLAMP_U8(x) (CAT_clamp(CAT_round(x), 0, 255))

uint32_t CAT_ZK_CO2()
{
	float co2 = CAT_clamp(readings.sunrise.ppm_filtered_uncompensated, 400, 4225);
	float t = CAT_inv_lerp(co2, 400, 4225);
	uint32_t final = ROUND_CLAMP_U8(t * 255.0f);
	CAT_printf("[ZK CO2] %f %f %d\n", co2, t, final);
	return final;
}

uint32_t CAT_ZK_PM2_5()
{
	uint32_t final = CAT_clamp(readings.sen5x.pm2_5, 0, 255);
	CAT_printf("[ZK PM2_5] %d\n", final);
	return final;
}

uint32_t CAT_ZK_temp()
{
	float temp = CAT_clamp(CAT_canonical_temp(), -100, 100);
	float t = CAT_inv_lerp(temp, -100, 100);
	uint32_t final = ROUND_CLAMP_U8(t * 255.0f);
	CAT_printf("[ZK TEMP] %d\n", final);
	return final;
}

uint32_t CAT_ZK_stroop()
{
	if(stroop_data_valid)
	{
		float t = CAT_clamp(stroop_correctness, 0, 1);
		uint32_t final = ROUND_CLAMP_U8(t * 255.0f);
		CAT_printf("[ZK STROOP] %d\n", final);
		return final;
	}
	CAT_printf("[ZK STROOP] NULL\n");
	return 0;
}

uint32_t CAT_ZK_survey()
{
	uint32_t final = survey_field;
	CAT_printf("[ZK SURVEY] %d\n", final);
	return survey_field;
}