#include "cat_aqi.h"

#include <stdio.h>
#include <math.h>

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
float CAT_temperature_score(float temperature) {
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
float CAT_humidity_score(float humidity) {
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
float CAT_co2_score(float co2) {
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
float CAT_pm25_score(float pm25) {
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
float CAT_nox_score(float nox) {
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
float CAT_voc_score(float voc) {
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
float CAT_iaq_score(float temperature, float humidity, float co2, float pm25, float nox, float voc) {
    // Calculate individual scores
    float tempScore = CAT_temperature_score(temperature);
    float humidityScore = CAT_humidity_score(humidity);
    float co2Score = CAT_co2_score(co2);
    float pm25Score = CAT_pm25_score(pm25);
    float noxScore = CAT_nox_score(nox);
    float vocScore = CAT_voc_score(voc);

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
float CAT_aq_aggregate_score()
{
    float temp = CAT_canonical_temp();
    float rh = readings.sen5x.humidity_rhpct;
    float co2 = readings.sunrise.ppm_filtered_compensated;
    float pm = readings.sen5x.pm2_5;
    float nox = readings.sen5x.nox_index;
    float voc = readings.sen5x.voc_index;
    float score = CAT_iaq_score(temp, rh, co2, pm, nox, voc);
	// It's the (5.0f - score) that makes this goodness instead of badness
    return ((5.0f - score) / 5.0f) * 100.0f;
}

const char* CAT_get_AQM_unit(int aqm)
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

float CAT_AQ_raw_scores[CAT_AQM_COUNT];
void CAT_AQ_store_raw_scores()
{
	CAT_AQ_raw_scores[CAT_AQM_CO2] = readings.sunrise.ppm_filtered_compensated;
	CAT_AQ_raw_scores[CAT_AQM_PM2_5] = readings.sen5x.pm2_5;
	CAT_AQ_raw_scores[CAT_AQM_VOC] = readings.sen5x.voc_index;
	CAT_AQ_raw_scores[CAT_AQM_NOX] = readings.sen5x.nox_index;
	CAT_AQ_raw_scores[CAT_AQM_TEMP] = CAT_canonical_temp();
	CAT_AQ_raw_scores[CAT_AQM_RH] = readings.sen5x.humidity_rhpct;
	CAT_AQ_raw_scores[CAT_AQM_AGGREGATE] = CAT_aq_aggregate_score();
}

float CAT_AQ_normalized_scores[CAT_AQM_COUNT];
void CAT_AQ_store_normalized_scores()
{
	CAT_AQ_normalized_scores[CAT_AQM_CO2] = CAT_co2_score(readings.sunrise.ppm_filtered_compensated);
	CAT_AQ_normalized_scores[CAT_AQM_PM2_5] = CAT_pm25_score(readings.sen5x.pm2_5);
	CAT_AQ_normalized_scores[CAT_AQM_VOC] = CAT_voc_score(readings.sen5x.voc_index);
	CAT_AQ_normalized_scores[CAT_AQM_NOX] = CAT_nox_score(readings.sen5x.nox_index);
	CAT_AQ_normalized_scores[CAT_AQM_TEMP] = CAT_temperature_score(CAT_canonical_temp());
	CAT_AQ_normalized_scores[CAT_AQM_RH] = CAT_humidity_score(readings.sen5x.humidity_rhpct);
	for(int i = 0; i < CAT_AQM_AGGREGATE; i++)
		CAT_AQ_normalized_scores[i] = inv_lerp(CAT_AQ_normalized_scores[i], 5, 0);

	CAT_AQ_normalized_scores[CAT_AQM_AGGREGATE] = CAT_aq_aggregate_score() / 100.0f;	
}