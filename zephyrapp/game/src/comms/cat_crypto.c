#include "cat_crypto.h"

#include "cat_math.h"
#include "cat_air.h"
#include "cat_persist.h"

/* PPM CO2 -> [0, 255]
	- Get normalized position in range [400, 4225] PPM
	- Clamp that position to [0, 1]
	- Map it to [0, 255]
*/
uint8_t CAT_ZKP_make_co2_byte()
{
	float t = CAT_inv_lerp(readings.sunrise.ppm_filtered_uncompensated, 400, 4225);
	return CAT_clamp(t*255, 0, 255);
}

/* ug/m3 PM 2.5 -> [0, 255]
	- Clamp ug/m3 to range [0, 255] ug/m3
*/
uint8_t CAT_ZKP_make_pm2_5_byte()
{
	return CAT_clamp(readings.sen5x.pm2_5, 0, 255);
}

/* Stroop response times -> [0, 255]
	- Get normalized position of each response time in range [0, 30] seconds
	- Clamp those positions to range [0, 1]
	- Get their mean
	- Map the mean to range [0, 255]
*/
uint8_t CAT_ZKP_make_stroop_time_byte()
{
	if(stroop_data_valid)
	{
		float t_cong = CAT_inv_lerp(stroop_data.mean_time_cong, 0, 30);
		float t_incong = CAT_inv_lerp(stroop_data.mean_time_incong, 0, 30);
		t_cong = CAT_clamp(t_cong, 0, 1);
		t_incong = CAT_clamp(t_incong, 0, 1);
		return CAT_clamp((t_cong + t_incong)*0.5f, 0, 255);
	}
	return 0;
}

/* Stroop throughput -> [0, 255]
	- Clamp responses/minute to the range [0, 255] responses/minute
*/
uint8_t CAT_ZKP_make_stroop_throughput_byte()
{
	if(stroop_data_valid)
	{
		return CAT_clamp(stroop_data.throughput, 0, 255);
	}
	return 0;
}