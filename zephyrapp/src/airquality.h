#pragma once
#include <stdint.h>

struct current_readings {
	struct {
		uint64_t uptime_last_updated;
		float temp, pressure;
	} lps22hh;
	
	struct {
		uint64_t uptime_last_updated;
		float ppm_filtered_compensated;
		float ppm_filtered_uncompensated;
		float temp;
	} sunrise;
	
	struct {
		uint64_t uptime_last_updated;
		float pm1_0, pm2_5, pm4_0, pm10_0;
		float nc0_5, nc1_0, nc2_5, nc4_0, nc10_0;
		float typ_particle_sz_um;
		float humidity_rhpct, temp_degC, voc_index, nox_index;
	} sen5x;
};

extern struct current_readings current_readings;
