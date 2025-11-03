#include "cat_air.h"

#include <stdio.h>
#include <math.h>
#include "cat_curves.h"
#include "cat_render.h"
#include "cat_persist.h"

static CAT_temperature_unit temperature_unit = CAT_TEMPERATURE_UNIT_DEGREES_CELSIUS;

CAT_temperature_unit CAT_AQ_get_temperature_unit()
{
	return temperature_unit;
}

void CAT_AQ_set_temperature_unit(CAT_temperature_unit unit)
{
	temperature_unit = unit;
}

float CAT_AQ_map_celsius(float temp)
{
	return
	temperature_unit == CAT_TEMPERATURE_UNIT_DEGREES_CELSIUS ?
	temp :
	temp * (9.0f / 5.0f) + 32;
}

const char* CAT_AQ_get_temperature_unit_string()
{
	return
	temperature_unit == CAT_TEMPERATURE_UNIT_DEGREES_CELSIUS ?
	"\3C" :
	"\3F";
}

float CAT_canonical_temp()
{
    return (readings.sen5x.temp_degC) ; // This is now correct and no adjustments are needed Leaving here for compatibility 
}

// Formula for WBGT from https://en.wikipedia.org/wiki/Wet-bulb_globe_temperature
// Approximation of T_w parameter from https://journals.ametsoc.org/view/journals/apme/50/11/jamc-d-11-0143.1.xml
// Lacking an on-board measurement for T_g parameter, we assume black globe temperature is roughly equal to dry air temperature
float CAT_wet_bulb_temp(float air_degc)
{
	float T_d = air_degc;
	float rh = readings.sen5x.humidity_rhpct;
	float T_w =
	T_d * atanf(0.151977 * powf(rh + 8.313659, 0.5f)) +
	atanf(T_d + rh) - atanf(rh - 1.676331) +
	(0.00391838 * powf(rh, 1.5f)) * atanf(0.023101 * rh) -
	4.686035;
	float T_g = T_d;
	return 0.7 * T_w + 0.1 * T_d + 0.2 * T_g;
}

/**
 * @brief Performs linear interpolation to calculate the score for a parameter.
 *
 * @param value The measurement value for the parameter.
 * @param breakpoints Array of breakpoint values for the parameter.
 * @param scores Array of corresponding scores at each breakpoint.
 * @param size Number of elements in the breakpoint and score arrays.
 * @return The interpolated score for the given value.
 */
float interpolate(float value, const float *breakpoints, const float *scores, int size) {
    // Handle values below the first breakpoint
    if (value <= breakpoints[0]) {
        return scores[0];
    }
    // Handle values above the last breakpoint
    if (value >= breakpoints[size - 1]) {
        return scores[size - 1];
    }
    // Perform linear interpolation
    for (int i = 0; i < size - 1; i++) {
        float lower = breakpoints[i];
        float upper = breakpoints[i + 1];
        if (value >= lower && value <= upper) {
            float scoreLower = scores[i];
            float scoreUpper = scores[i + 1];
            float interpolatedScore = scoreLower + (scoreUpper - scoreLower) * (value - lower) / (upper - lower);
            return interpolatedScore;
        }
    }
    // Should not reach here
    return 0.0f;
}

/**
 * @brief Calculates the temperature score based on the temperature value.
 *
 * @param temperature Temperature in °C.
 * @return The temperature score.
 */
float CAT_temp_score(float temperature) {
    // Temperature (°C) breakpoints and scores
    const float tempBreakpoints[] = { 0.0f, 8.0f, 16.0f, 18.0f, 20.0f, 25.0f, 27.0f, 29.0f, 34.0f, 35.0f };
    const float tempScores[]      = { 5.0f, 5.0f, 4.0f,  3.0f,  1.0f, 0.0f,  1.0f,  3.0f,  4.0f, 5.0f };
    const int tempSize = sizeof(tempBreakpoints) / sizeof(tempBreakpoints[0]);
    return interpolate(temperature, tempBreakpoints, tempScores, tempSize);
}

/**
 * @brief Calculates the humidity score based on the relative humidity value.
 *
 * @param humidity Relative Humidity in %.
 * @return The humidity score.
 */
float CAT_RH_score(float humidity) {
    // Relative Humidity (%) breakpoints and scores
    const float humidityBreakpoints[] = { 0.0f, 14.0f, 23.0f, 30.0f, 40.0f, 50.0f, 60.0f, 65.0f, 80.0f, 85.0f };
    const float humidityScores[]      = { 5.0f, 5.0f, 4.0f,   3.0f,  1.0f, 0.0f,  1.0f,  3.0f,  4.0f, 5.0f };
    const int humiditySize = sizeof(humidityBreakpoints) / sizeof(humidityBreakpoints[0]);
    return interpolate(humidity, humidityBreakpoints, humidityScores, humiditySize);
}

/**
 * @brief Calculates the CO2 score based on the CO2 concentration.
 *
 * @param co2 CO₂ concentration in ppm.
 * @return The CO2 score.
 */
float CAT_CO2_score(float co2) {
    // CO₂ (ppm) breakpoints and scores
    const float co2Breakpoints[] = { 0.0f, 420.0f, 800.0f, 1000.0f, 1400.0f, 4500.0f };
    const float co2Scores[]      = { 0.0f, 0.0f,   1.0f,    2.0f,    3.0f,    5.0f };
    const int co2Size = sizeof(co2Breakpoints) / sizeof(co2Breakpoints[0]);
    return interpolate(co2, co2Breakpoints, co2Scores, co2Size);
}

/**
 * @brief Calculates the PM2.5 score based on the PM2.5 concentration.
 *
 * @param pm25 PM2.5 concentration in μg/m³.
 * @return The PM2.5 score.
 */
float CAT_PM2_5_score(float pm25) {
    // PM2.5 (μg/m³) breakpoints and scores
    const float pm25Breakpoints[] = { 0.0f, 5.0f, 12.0f, 35.0f, 55.0f, 150.0f };
    const float pm25Scores[]      = { 0.0f, 1.0f, 2.0f,  3.0f,  4.0f,  5.0f };
    const int pm25Size = sizeof(pm25Breakpoints) / sizeof(pm25Breakpoints[0]);
    return interpolate(pm25, pm25Breakpoints, pm25Scores, pm25Size);
}

/**
 * @brief Calculates the NOx score based on the NOx index.
 *
 * @param nox NOx Index.
 * @return The NOx score.
 */
float CAT_NOX_score(float nox) {
    // NOx Index breakpoints and scores
    const float noxBreakpoints[] = { 0.0f, 1.0f, 90.0f, 150.0f, 300.0f, 310.0f };
    const float noxScores[]      = { 0.0f, 0.5f,   2.0f,   3.0f,   4.0f,   5.0f };
    const int noxSize = sizeof(noxBreakpoints) / sizeof(noxBreakpoints[0]);
    return interpolate(nox, noxBreakpoints, noxScores, noxSize);
}

/**
 * @brief Calculates the VOC score based on the VOC index.
 *
 * @param voc VOC Index.
 * @return The VOC score.
 */
float CAT_VOC_score(float voc) {
    // VOC Index breakpoints and scores
    const float vocBreakpoints[] = { 0.0f, 100.0f, 150.0f, 250.0f, 400.0f, 410.0f };
    const float vocScores[]      = { 0.0f, 0.5f,    2.0f,   3.0f,   4.0f,   5.0f };
    const int vocSize = sizeof(vocBreakpoints) / sizeof(vocBreakpoints[0]);
    return interpolate(voc, vocBreakpoints, vocScores, vocSize);
}

/**
 * @brief Calculates the Indoor Air Quality (IAQ) score based on various environmental parameters.
 *
 * @param temperature Temperature in °C.
 * @param humidity Relative Humidity in %.
 * @param co2 CO₂ concentration in ppm.
 * @param pm25 PM2.5 concentration in μg/m³.
 * @param nox NOx Index.
 * @param voc VOC Index.
 * @return The final IAQ score.
 */
float CAT_IAQ_score(float temperature, float humidity, float co2, float pm25, float nox, float voc) {
    // Calculate individual scores
    float tempScore = CAT_temp_score(temperature);
    float humidityScore = CAT_RH_score(humidity);
    float co2Score = CAT_CO2_score(co2);
    float pm25Score = CAT_PM2_5_score(pm25);
    float noxScore = CAT_NOX_score(nox);
    float vocScore = CAT_VOC_score(voc);

    // Step 2: Determine Base IAQ Score (maximum of pollutant scores)
    float pollutantScores[] = { co2Score, pm25Score, noxScore, vocScore };
    float baseScore = pollutantScores[0];
    for (int i = 1; i < 4; i++) {
        if (pollutantScores[i] > baseScore) {
            baseScore = pollutantScores[i];
        }
    }

    // Step 3: Apply Penalties for Multiple Poor Parameters
    // A "bad" score is greater than 3.0
    int badPollutantCount = 0;
    for (int i = 0; i < 4; i++) {
        if (pollutantScores[i] > 3.0f) {
            badPollutantCount++;
        }
    }

    const float penaltyFactor = 0.5f;
    if (badPollutantCount > 1) {
        baseScore += (badPollutantCount - 1) * penaltyFactor;
    }
    if (baseScore > 5.0f) {
        baseScore = 5.0f;
    }

    // Step 4: Adjust for Temperature and Humidity
    const float multiplierFactor = 0.1f;
    float tempMultiplier = 1.0f;
    float humidityMultiplier = 1.0f;

    if (tempScore > 1.0f) {
        tempMultiplier += (tempScore - 1.0f) * multiplierFactor;
    }

    if (humidityScore > 1.0f) {
        humidityMultiplier += (humidityScore - 1.0f) * multiplierFactor;
    }

    float finalScore = baseScore * tempMultiplier * humidityMultiplier;
    if (finalScore > 5.0f) {
        finalScore = 5.0f;
    }

    // Step 5: Round the Final Score to two decimal places
    finalScore = ((int)(finalScore * 100.0f + 0.5f)) / 100.0f;

    return finalScore;
}

// This is a measure of goodness rather than badness,
// a somewhat confusing choice given convention. Sorry!
float CAT_AQ_aggregate_score()
{
    float temp = CAT_canonical_temp();
    float rh = readings.sen5x.humidity_rhpct;
    float co2 = readings.sunrise.ppm_filtered_compensated;
    float pm = readings.sen5x.pm2_5;
    float nox = readings.sen5x.nox_index;
    float voc = readings.sen5x.voc_index;
    float score = CAT_IAQ_score(temp, rh, co2, pm, nox, voc);
	// It's the (5.0f - score) that makes this goodness instead of badness
    return ((5.0f - score) / 5.0f) * 100.0f;
}

static const char* title_strings[] =
{
	[CAT_AQM_CO2] = "CO2",
	[CAT_AQM_PM2_5] = "PM",
	[CAT_AQM_NOX] = "NOX",
	[CAT_AQM_VOC] = "VOC",
	[CAT_AQM_TEMP] = "TEMP",
	[CAT_AQM_RH] = "RH",
	[CAT_AQM_AGGREGATE] = "\4CritAQ Score"
};

const char* CAT_AQ_title_string(int aqm)
{
	return title_strings[aqm];
}

const char* CAT_AQ_unit_string(int aqm)
{
	switch(aqm)
	{
		case CAT_AQM_CO2: return "ppm";
		case CAT_AQM_PM2_5: return "\4g/m\5";
		case CAT_AQM_TEMP: return CAT_AQ_get_temperature_unit_string();
		case CAT_AQM_RH : return "\%";
		default: return "";
	}
}

static const char* grade_strings[] =
{
	"F",
	"D-",
	"D",
	"D+",
	"C-",
	"C",
	"C+",
	"B-",
	"B",
	"B+",
	"A-",
	"A",
	"A+"
};

const char* CAT_AQ_grade_string(float score)
{
	int grades = sizeof(grade_strings)/sizeof(grade_strings[0]);
	int index = quantize(score, 1, grades);
	return grade_strings[index];
}

static uint16_t grade_colours[] = 
{
	CAT_GRADE_COLOUR_BAD,
	CAT_GRADE_COLOUR_MID,
	CAT_GRADE_COLOUR_GOOD
};

uint16_t CAT_AQ_grade_colour(float score)
{
	return CAT_colour_curve(grade_colours, 3, score);
}

float raw_values[CAT_AQM_COUNT];
float raw_values_last[CAT_AQM_COUNT];
float normalized_scores[CAT_AQM_COUNT];
float deltas[CAT_AQM_COUNT];

void CAT_AQ_poll_readings()
{
	if(!CAT_AQ_sensors_initialized())
	{
		if(CAT_logs_initialized())
		{
			CAT_log_cell last_cell;
			CAT_read_log_cell_at_idx(CAT_get_log_cell_count()-1, &last_cell);

			raw_values[CAT_AQM_CO2] = last_cell.co2_ppmx1 * 1.0f;
			raw_values[CAT_AQM_PM2_5] = last_cell.pm_ugmx100[1] / 100.0f;
			raw_values[CAT_AQM_VOC] = last_cell.voc_index;
			raw_values[CAT_AQM_NOX] = last_cell.nox_index;
			raw_values[CAT_AQM_TEMP] = last_cell.temp_Cx1000 / 1000.0f;
			raw_values[CAT_AQM_RH] = last_cell.rh_pctx100 / 100.0f;

			float iaq = CAT_IAQ_score
			(
				raw_values[CAT_AQM_TEMP],
				raw_values[CAT_AQM_RH],
				raw_values[CAT_AQM_CO2],
				raw_values[CAT_AQM_PM2_5],
				raw_values[CAT_AQM_NOX],
				raw_values[CAT_AQM_VOC]
			);
			raw_values[CAT_AQM_AGGREGATE] = ((5.0f - iaq) / 5.0f) * 100.0f;

			for(int i = 0; i < CAT_AQM_COUNT; i++)
				raw_values_last[i] = raw_values[i];
		}
	}
	else
	{
		memcpy(raw_values_last, raw_values, sizeof(raw_values));

		raw_values[CAT_AQM_CO2] = readings.sunrise.ppm_filtered_compensated;
		raw_values[CAT_AQM_PM2_5] = readings.sen5x.pm2_5;
		raw_values[CAT_AQM_VOC] = readings.sen5x.voc_index;
		raw_values[CAT_AQM_NOX] = readings.sen5x.nox_index;
		raw_values[CAT_AQM_TEMP] = CAT_canonical_temp();
		raw_values[CAT_AQM_RH] = readings.sen5x.humidity_rhpct;
		raw_values[CAT_AQM_AGGREGATE] = CAT_AQ_aggregate_score();
	}

	for(int i = 0; i <= CAT_AQM_AGGREGATE; i++)
	{
		float delta = raw_values[i] - raw_values_last[i];
		if(delta != 0)
			deltas[i] = delta;
	}

	normalized_scores[CAT_AQM_CO2] = CAT_CO2_score(raw_values[CAT_AQM_CO2]);
	normalized_scores[CAT_AQM_PM2_5] = CAT_PM2_5_score(raw_values[CAT_AQM_PM2_5]);
	normalized_scores[CAT_AQM_VOC] = CAT_VOC_score(raw_values[CAT_AQM_VOC]);
	normalized_scores[CAT_AQM_NOX] = CAT_NOX_score(raw_values[CAT_AQM_NOX]);
	normalized_scores[CAT_AQM_TEMP] = CAT_temp_score(raw_values[CAT_AQM_TEMP]);
	normalized_scores[CAT_AQM_RH] = CAT_RH_score(raw_values[CAT_AQM_RH]);
	for(int i = 0; i < CAT_AQM_AGGREGATE; i++)
		normalized_scores[i] = inv_lerp(normalized_scores[i], 5, 0);
	normalized_scores[CAT_AQM_AGGREGATE] = raw_values[CAT_AQM_AGGREGATE] / 100.0f;	
}

float CAT_AQ_value(int aqm)
{
	return raw_values[aqm];
}

float CAT_AQ_score(int aqm)
{
	return normalized_scores[aqm];
}

float CAT_AQ_delta(int aqm)
{
	return deltas[aqm];
}

float CAT_AQ_block_value(CAT_AQ_score_block* block, int aqm)
{
	switch(aqm)
	{
		case CAT_AQM_CO2: return block->CO2;
		case CAT_AQM_PM2_5: return block->PM2_5 / 100.0f;
		case CAT_AQM_NOX: return block->NOX;
		case CAT_AQM_VOC: return block->VOC;
		case CAT_AQM_TEMP: return block->temp / 1000.0f;
		case CAT_AQM_RH : return block->rh / 100.0f;
		case CAT_AQM_AGGREGATE: return block->aggregate;
		default: return 0;
	}
}

float CAT_AQ_block_score(CAT_AQ_score_block* block, int aqm)
{
	switch(aqm)
	{
		case CAT_AQM_CO2: return inv_lerp(CAT_CO2_score(block->CO2), 5, 0);
		case CAT_AQM_PM2_5: return inv_lerp(CAT_PM2_5_score(block->PM2_5 / 100.0f), 5, 0);
		case CAT_AQM_NOX: return inv_lerp(CAT_NOX_score(block->NOX), 5, 0);
		case CAT_AQM_VOC: return inv_lerp(CAT_VOC_score(block->VOC), 5, 0);
		case CAT_AQM_TEMP: return inv_lerp(CAT_temp_score(block->temp / 1000.0f), 5, 0);
		case CAT_AQM_RH : return inv_lerp(CAT_RH_score(block->rh / 100.0f), 5, 0);
		case CAT_AQM_AGGREGATE: return block->aggregate / 100.0f;
		default: return 0;
	}
}

int float2int(float f, int scale_factor)
{
	return roundf(f * scale_factor);
}

float int2float(int i, float scale_factor)
{
	return i / scale_factor;
}

int move_average(int x_bar, float x, float scale_factor)
{
	float x_bar_f = int2float(x_bar, scale_factor);
	float n = aq_moving_score_samples+1;
	float old_weight = (n-1) / n;
	float new_weight = 1.0f / n;
	x_bar_f = x_bar_f * old_weight + x * new_weight;
	return float2int(x_bar_f, scale_factor);
}

bool CAT_AQ_moving_scores_initialized()
{
	return aq_moving_score_time > 0;
}

void CAT_AQ_update_moving_scores()
{
	CAT_AQ_score_block* block = &aq_moving_scores;
	block->CO2 = move_average(block->CO2, readings.sunrise.ppm_filtered_uncompensated, 1);
	block->NOX = move_average(block->NOX, readings.sen5x.nox_index, 1);
	block->VOC = move_average(block->VOC, readings.sen5x.voc_index, 1);
	block->PM2_5 = move_average(block->PM2_5, readings.sen5x.pm2_5, 100);
	block->temp = move_average(block->temp, CAT_canonical_temp(), 1000);
	block->rh = move_average(block->rh, readings.sen5x.humidity_rhpct, 100);
	block->aggregate = move_average(block->aggregate, CAT_AQ_aggregate_score(), 1);
	
	aq_moving_score_samples += 1;
	aq_moving_score_time = CAT_get_RTC_now();
}

bool CAT_AQ_weekly_scores_initialized()
{
	return aq_weekly_score_time > 0;
}

void CAT_AQ_push_weekly_scores(CAT_AQ_score_block* in)
{
	if(aq_weekly_score_time == 0)
	{
		for(int i = 0; i < 7; i++)
			memcpy(&aq_weekly_scores[i], &aq_moving_scores, sizeof(CAT_AQ_score_block));
		aq_weekly_score_head = 0;
	}

	CAT_AQ_score_block* block = &aq_weekly_scores[aq_weekly_score_head];
	memcpy(block, in, sizeof(CAT_AQ_score_block));

	aq_weekly_score_head = (aq_weekly_score_head+1) % 7;
	aq_weekly_score_time = CAT_get_RTC_now();
	aq_moving_score_samples = 0;
}

CAT_AQ_score_block* CAT_AQ_get_weekly_scores(int idx)
{
	idx = (aq_weekly_score_head+idx) % 7;
	return &aq_weekly_scores[idx];
}

void CAT_AQ_tick()
{
	uint64_t now = CAT_get_RTC_now();

	CAT_AQ_poll_readings();

	uint64_t time_since = now - aq_moving_score_time;
	if(time_since > CAT_AQ_MOVING_SAMPLE_PERIOD && CAT_AQ_sensors_initialized())
		CAT_AQ_update_moving_scores();
	
	time_since = now - aq_weekly_score_time;
	if(time_since > CAT_AQ_SPARKLINE_SAMPLE_PERIOD && CAT_AQ_sensors_initialized())
		CAT_AQ_push_weekly_scores(&aq_moving_scores);
}
