#include "cat_core.h"

#include <stdio.h>
#include <math.h>

// Helper function for interpolation
float interpolate(float value, float breakpoints[], float scores[], int length) {
    if (value <= breakpoints[0]) return scores[0];
    if (value >= breakpoints[length - 1]) return scores[length - 1];
    
    for (int i = 1; i < length; i++) {
        if (value < breakpoints[i]) {
            float delta = (value - breakpoints[i - 1]) / (breakpoints[i] - breakpoints[i - 1]);
            return scores[i - 1] + delta * (scores[i] - scores[i - 1]);
        }
    }
    return scores[length - 1];
}

// Score functions for different pollutants and factors
float CAT_co2_score(float co2) {
    float breakpoints[] = {0, 600, 1000, 2000, 4500};
    float scores[] = {0, 1, 2, 3, 4};
    return interpolate(co2, breakpoints, scores, 5);
}

float CAT_voc_score(float voc) {
    float breakpoints[] = {1, 75, 150, 250, 400, 500};
    float scores[] = {0, 1, 2, 3, 4, 5};
    return interpolate(voc, breakpoints, scores, 6);
}

float CAT_nox_score(float nox) {
    float breakpoints[] = {1, 20, 90, 150, 300, 500};
    float scores[] = {0, 1, 2, 3, 4, 5};
    return interpolate(nox, breakpoints, scores, 6);
}

float CAT_pm25_score(float pm25) {
    float breakpoints[] = {0, 12, 35, 55, 150};
    float scores[] = {0, 1, 2, 3, 4};
    return interpolate(pm25, breakpoints, scores, 5);
}

float CAT_mean_temp()
{
	return (aqi.lps22hh.temp + aqi.sunrise.temp + aqi.sen5x.temp_degC) / 3.0f;
}

float CAT_temperature_score(float temp) {
    if (temp < -50 || temp > 50) return 5;  // Assign worst score if temp is out of range
    float breakpoints[] = {-50, -20, 0, 10, 20, 25, 30, 40, 50};
    float scores[] = {5, 4, 3, 2, 1, 2, 3, 4, 5};
    return interpolate(temp, breakpoints, scores, 9);
}

float CAT_humidity_score(float rh) {
    if (rh < 0 || rh > 100) return 5;  // Assign worst score if humidity is out of range
    float breakpoints[] = {0, 14, 23, 30, 40, 50, 60, 65, 80, 100};
    float scores[] = {5, 4, 3, 2, 1, 2, 3, 4, 5, 5};
    return interpolate(rh, breakpoints, scores, 10);
}

// Calculate penalty based on critical scores
float calculate_penalty(float pollutant_scores[], int length, float temp_score, float rh_score) {
    int critical_count = 0;
    float severity_factor = 0;

    for (int i = 0; i < length; i++) {
        if (pollutant_scores[i] >= 3) {
            critical_count++;
            severity_factor += pollutant_scores[i] - 2;  // Severity adjustment
        }
    }

    float penalty = (critical_count > 0) ? (exp(critical_count) - 1) * (1 + 0.1 * severity_factor) : 0;

    // Additional penalties for extreme temperature and humidity if they are critical
    if (temp_score >= 4) {
        penalty += exp(1) - 1;
    }
    if (rh_score >= 4) {
        penalty += exp(1) - 1;
    }

    // Cap penalty to a maximum value of 5
    return (penalty > 5) ? 5 : penalty;
}

// Main function to calculate IAQ score
float CAT_iaq_score(float co2, float voc, float nox, float pm25, float temp, float rh) {
    float pollutant_scores[4];
    pollutant_scores[0] = CAT_co2_score(co2);
    pollutant_scores[1] = CAT_voc_score(voc);
    pollutant_scores[2] = CAT_nox_score(nox);
    pollutant_scores[3] = CAT_pm25_score(pm25);

    float temp_score = CAT_temperature_score(temp);
    float rh_score = CAT_humidity_score(rh);

    // Base score is determined by the worst pollutant
    float base_score = 0;
    for (int i = 0; i < 4; i++) {
        if (pollutant_scores[i] > base_score) {
            base_score = pollutant_scores[i];
        }
    }

    // Calculate penalty
    float penalty = calculate_penalty(pollutant_scores, 4, temp_score, rh_score);

    // Final IAQ score capped at 5
    float final_score = base_score + penalty;
    if (final_score > 5) {
        final_score = 5;
    }

    // Ensure the score is between 0 and 5
    return (final_score < 0) ? 0 : final_score;
}

void CAT_AQI_quantize(int* temp_idx, int* co2_idx, int* pm_idx, int* voc_idx, int* nox_idx)
{
	float co2s = CAT_co2_score(aqi.sunrise.ppm_filtered_compensated);
	float pms = CAT_pm25_score(aqi.sen5x.pm2_5);
	float vocs = CAT_voc_score(aqi.sen5x.voc_index);
	float noxs = CAT_nox_score(aqi.sen5x.nox_index);
	*co2_idx = quantize(co2s, 5, 3);
	*pm_idx = quantize(pms, 5, 3);
	*voc_idx = quantize(vocs, 5, 3);
	*nox_idx = quantize(noxs, 5, 3);

	float ts = CAT_temperature_score(CAT_mean_temp());
	if(CAT_mean_temp < 20 && ts >= 3)
		*temp_idx = 0;
	else if(ts >= 3)
		*temp_idx = 2;
	*temp_idx = 1;
}