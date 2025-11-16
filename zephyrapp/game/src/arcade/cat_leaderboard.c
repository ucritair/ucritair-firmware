#include "cat_leaderboard.h"

#include <string.h>
#include "cat_core.h"
#include "cat_air.h"

int snake_highscore = 0;
int mines_highscore = 0;
int foursquares_highscore = 0;

CAT_stroop_data stroop_data = {0};
bool stroop_data_valid = false;
float stroop_correctness = 0;

uint8_t survey_mask;

uint32_t CAT_ZK_timestamp()
{
	return CAT_get_RTC_now();
}

uint32_t CAT_ZK_CO2()
{
	
	return CAT_f2u32(readings.sunrise.ppm_filtered_uncompensated);
}

uint32_t CAT_ZK_PM2_5()
{
	return CAT_f2u32(readings.sen5x.pm2_5);
}

uint32_t CAT_ZK_temp()
{
	return CAT_f2u32(CAT_canonical_temp());	
}

uint32_t CAT_ZK_stroop()
{
	return CAT_f2u32(stroop_correctness);
}

uint32_t CAT_ZK_survey()
{
	return survey_mask;
}

