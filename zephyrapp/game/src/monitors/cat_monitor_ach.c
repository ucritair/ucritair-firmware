#include "cat_monitor_ach.h"

#include <stdint.h>
#include "cat_core.h"
#include <math.h>
#include "cat_math.h"

#define VIEW_CO2 0
#define VIEW_PN_10_0 1

struct dp
{
	uint64_t time;
	int ppm;
};

float CAT_monitor_graph_calculate_ACH(int view, int16_t* values, uint64_t* timestamps, int32_t* indices, int start, int end)
{
	int cursor_start = start;
	int cursor_end = end;
	struct dp max = {.ppm=INT16_MIN};
	struct dp min = {.ppm=INT16_MAX};
	
	for (int cursor = start; cursor <= end; cursor++)
	{
		int idx = indices[cursor];
		if (idx == -1)
			continue;
		struct dp here = {.time = timestamps[cursor], .ppm = values[cursor]};

		if (here.ppm > max.ppm)
		{
			max = here;
			cursor_start = cursor;
		}
		if (here.ppm < min.ppm)
		{
			min = here;
			cursor_end = cursor;
		}
	}
	double decay_concentration = (double) min.ppm + ((double) max.ppm - (double) min.ppm) / M_E;
    
    // Decay time (Assume for simplicity it's between max and baseline times)
   // <A FUNCTION THAT GIVES ME THE TIME that the concentration dropped below decay_concentration and returns decay_time)

	uint64_t decay_time = 0;
	for (int cursor = cursor_start; cursor <= cursor_end; cursor++)
	{
		int idx = indices[cursor];
		if (idx == -1)
			continue;

		if(values[cursor] < decay_concentration)
		{
			decay_time = timestamps[cursor];
			break;
		}
	}

	if (decay_time == 0 || decay_time==max.time || max.ppm == min.ppm || max.ppm == 0 || min.ppm == 99999)
	{
		return -1;
	}

    // Calculate ACH
    float ach = 1.0 / ((double)(decay_time - max.time) / 3600.0);  // Convert to hours
	return ach;
}
