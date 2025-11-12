#include "cat_crypto.h"

#include "cat_math.h"
#include "cat_air.h"
#include "cat_persist.h"

#define CO2_MIN 400
#define CO2_MAX 4225

uint8_t CAT_ZKP_make_co2_byte()
{
	float t = CAT_inv_lerp(readings.sunrise.ppm_filtered_uncompensated, CO2_MIN, CO2_MAX);
	t = CAT_clamp(t, 0, 1);
	return t * 255;
}

uint8_t CAT_ZKP_make_pm2_5_byte()
{
	return CAT_clamp(readings.sen5x.pm2_5, 0, 255);
}

#define CONG_BASELINE
#define CONG_MAX
#define INCONG_BASELINE
#define INCONG_MAX

uint8_t CAT_ZKP_make_stroop_time_byte()
{
	if(stroop_data_valid)
	{
		float cong = stroop_data.mean_time_cong;
		float incong = stroop_data.mean_time_incong;
	}
	return 0;
}

#define THRPT_BASELINE
#define THRPT_MAX

uint8_t CAT_ZKP_make_stroop_throughput_byte()
{
	if(stroop_data_valid)
	{

	}
	return 0;
}