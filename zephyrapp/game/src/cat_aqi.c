#include "cat_aqi.h"

#include <stdio.h>
#include <math.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
// CRISES

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

float CAT_canonical_temp()
{
	return (readings.sen5x.temp_degC - 2.75) ;
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
float CAT_rh_score(float humidity) {
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
    float humidityScore = CAT_rh_score(humidity);
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

const char* CAT_get_AQM_unit_string(int aqm)
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

float raw_scores[CAT_AQM_COUNT];

float CAT_AQ_get_raw_score(int aqm)
{
	return raw_scores[aqm];
}

void CAT_AQ_store_raw_scores()
{
	raw_scores[CAT_AQM_CO2] = readings.sunrise.ppm_filtered_compensated;
	raw_scores[CAT_AQM_PM2_5] = readings.sen5x.pm2_5;
	raw_scores[CAT_AQM_VOC] = readings.sen5x.voc_index;
	raw_scores[CAT_AQM_NOX] = readings.sen5x.nox_index;
	raw_scores[CAT_AQM_TEMP] = CAT_canonical_temp();
	raw_scores[CAT_AQM_RH] = readings.sen5x.humidity_rhpct;
	raw_scores[CAT_AQM_AGGREGATE] = CAT_AQ_aggregate_score();
}

float normalized_scores[CAT_AQM_COUNT];

float CAT_AQ_get_normalized_score(int aqm)
{
	return normalized_scores[aqm];
}

void CAT_AQ_store_normalized_scores()
{
	normalized_scores[CAT_AQM_CO2] = CAT_CO2_score(readings.sunrise.ppm_filtered_compensated);
	normalized_scores[CAT_AQM_PM2_5] = CAT_PM2_5_score(readings.sen5x.pm2_5);
	normalized_scores[CAT_AQM_VOC] = CAT_VOC_score(readings.sen5x.voc_index);
	normalized_scores[CAT_AQM_NOX] = CAT_NOX_score(readings.sen5x.nox_index);
	normalized_scores[CAT_AQM_TEMP] = CAT_temp_score(CAT_canonical_temp());
	normalized_scores[CAT_AQM_RH] = CAT_rh_score(readings.sen5x.humidity_rhpct);
	for(int i = 0; i < CAT_AQM_AGGREGATE; i++)
		normalized_scores[i] = inv_lerp(normalized_scores[i], 5, 0);

	normalized_scores[CAT_AQM_AGGREGATE] = CAT_AQ_aggregate_score() / 100.0f;	
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// CRISES

static const float crisis_severity_cutoffs[] =
{
	// NONE, MILD, MODERATE, SEVERE
	1.0f, 0.5f, 0.35f, 0.25f,
};

static const int crisis_primetimes[] =
{
	0, // NONE
	CAT_MINUTE_SECONDS * 30, // MILD
	CAT_MINUTE_SECONDS * 20, // MODERATE
	CAT_MINUTE_SECONDS * 10 // SEVERE
};

CAT_AQ_crisis_type CAT_AQ_poll_crisis()
{
	if(!CAT_is_AQ_initialized())
		return CAT_AQ_CRISIS_TYPE_NONE;

	int worst_aqm = -1;
	float worst_score = __FLT_MAX__;
	for(int i = 0; i < CAT_AQM_AGGREGATE; i++)
	{
		float score = normalized_scores[i];
		if(score < 0.5f && score < worst_score)
		{
			worst_aqm = i;
			worst_score = score;
		}
	}
		
	switch(worst_aqm)
	{
		case CAT_AQM_CO2: return CAT_AQ_CRISIS_TYPE_CO2;
		case CAT_AQM_PM2_5: return CAT_AQ_CRISIS_TYPE_PM2_5;
		case CAT_AQM_NOX:
		case CAT_AQM_VOC: return CAT_AQ_CRISIS_TYPE_NOX_VOC;
		case CAT_AQM_TEMP:
		case CAT_AQM_RH: return CAT_AQ_CRISIS_TYPE_TEMP_RH;
		default: return CAT_AQ_CRISIS_TYPE_NONE;
	};
}

float get_crisis_score(CAT_AQ_crisis_type type)
{
	switch(type)
	{
		case CAT_AQ_CRISIS_TYPE_CO2: return normalized_scores[CAT_AQM_CO2];
		case CAT_AQ_CRISIS_TYPE_PM2_5: return normalized_scores[CAT_AQM_PM2_5];
		case CAT_AQ_CRISIS_TYPE_NOX_VOC: return min(normalized_scores[CAT_AQM_NOX], normalized_scores[CAT_AQM_VOC]);
		case CAT_AQ_CRISIS_TYPE_TEMP_RH: return min(normalized_scores[CAT_AQM_TEMP], normalized_scores[CAT_AQM_RH]);
		default: return 1.0f;
	}
};

CAT_AQ_crisis_severity CAT_AQ_poll_crisis_severity(CAT_AQ_crisis_type type)
{	
	float score = get_crisis_score(type);

	for(int s = CAT_AQ_CRISIS_SEVERITY_EXTREME; s >= CAT_AQ_CRISIS_SEVERITY_NONE; s--)
	{
		float cutoff = crisis_severity_cutoffs[s];
		if(score <= cutoff)
			return s;
	}
	
	return CAT_AQ_CRISIS_SEVERITY_NONE;
}

static CAT_AQ_crisis_type crisis_type = CAT_AQ_CRISIS_TYPE_NONE;
static CAT_AQ_crisis_severity crisis_severity = CAT_AQ_CRISIS_SEVERITY_NONE;
static float crisis_timer = 0;
static CAT_AQ_crisis_response_type crisis_response_type;
static bool crisis_notice = false;

void CAT_AQ_start_crisis(CAT_AQ_crisis_type type, CAT_AQ_crisis_severity severity)
{
	crisis_type = type;
	crisis_severity = severity;
	crisis_timer = 0;
	crisis_response_type = CAT_AQ_CRISIS_RESPONSE_TYPE_NONE;
	crisis_notice = true;
}

void CAT_AQ_stop_crisis(CAT_AQ_crisis_response_type response_type)
{
	crisis_response_type = response_type;
}

bool CAT_AQ_is_crisis_ongoing()
{
	return crisis_response_type = CAT_AQ_CRISIS_RESPONSE_TYPE_NONE;
}

void CAT_AQ_crisis_tick()
{
	if(crisis_type != CAT_AQ_CRISIS_TYPE_NONE && CAT_AQ_is_crisis_ongoing())
	{
		crisis_timer += CAT_get_delta_time_s();
		if(CAT_AQ_poll_crisis_severity(crisis_type) == CAT_AQ_CRISIS_SEVERITY_NONE)
			CAT_AQ_stop_crisis(CAT_AQ_CRISIS_RESPONSE_TYPE_AUTOMATIC);
	}
}

int CAT_AQ_get_crisis_primetime()
{
	return crisis_primetimes[crisis_severity];
}

int CAT_AQ_get_crisis_duration()
{
	return crisis_timer;
}

int CAT_AQ_get_crisis_overtime()
{
	return crisis_timer - CAT_AQ_get_crisis_primetime();
}

CAT_AQ_crisis_response_grade CAT_AQ_grade_crisis_response()
{
	int primetime = CAT_AQ_get_crisis_primetime();
	int overtime = crisis_timer - primetime;
	
	if(overtime <= 0)
	{
		if(crisis_timer <= CAT_MINUTE_SECONDS*5)
			return CAT_AQ_CRISIS_RESPONSE_GRADE_EXCELLENT;
		return CAT_AQ_CRISIS_RESPONSE_GRADE_ADEQUATE;
	}
	else if(overtime <= primetime * 2)
		return CAT_AQ_CRISIS_RESPONSE_GRADE_INADEQUATE;
	else
		return CAT_AQ_CRISIS_RESPONSE_GRADE_DISASTROUS;
}

bool CAT_AQ_is_crisis_notice_posted()
{
	return
	CAT_AQ_is_crisis_ongoing() &&
	crisis_notice;
}

void CAT_AQ_dismiss_crisis_notice()
{
	crisis_notice = false;
}

static const char* crisis_titles[] =
{
	"N/A",
	"MIASMA",
	"PARTICULATE STORM",
	"REAGENT STORM",
	"THERMICS"
};

static const char* crisis_severity_strings[] =
{
	"N/A",
	"MILD",
	"MODERATE",
	"SEVERE",
};

const char* CAT_AQ_get_crisis_title()
{
	return crisis_titles[crisis_type];
}

const char* CAT_AQ_get_crisis_severity_string()
{
	return crisis_severity_strings[crisis_severity];
}

void CAT_AQ_tick()
{
	CAT_AQ_store_raw_scores();
	CAT_AQ_store_normalized_scores();

	if(!CAT_AQ_is_crisis_ongoing())
	{
		CAT_AQ_crisis_type type = CAT_AQ_poll_crisis();
		if(type != CAT_AQ_CRISIS_TYPE_NONE)
		{
			CAT_AQ_crisis_severity severity = CAT_AQ_poll_crisis_severity(type);
			if(severity != CAT_AQ_CRISIS_SEVERITY_NONE)
				CAT_AQ_start_crisis(type, severity);
		}
	}
	else
	{
		CAT_AQ_crisis_tick();	
	}
}