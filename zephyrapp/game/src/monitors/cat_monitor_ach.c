#include "cat_monitor_ach.h"

#include <stdint.h>
#include "cat_core.h"
#include <math.h>
#include "cat_math.h"

static int view;
static int16_t* values;
static uint64_t* timestamps;
static int32_t* indices;
static int extent;

static int cursor_start;
static int cursor_end;

void CAT_monitor_graph_set_ACH_data(int _view, int16_t* _values, uint64_t* _timestamps, int32_t* _indices, int _extent)
{
	view = _view;
	values = _values;
	timestamps = _timestamps;
	indices = _indices;
	extent = _extent;
}

void CAT_monitor_graph_auto_ACH_cursors(int* start, int* end)
{
	int16_t peak = INT16_MIN;
	int peak_idx = -1;
	for(int i = 0; i < extent; i++)
	{
		if(indices[i] == -1)
			continue;

		int16_t value = values[i];
		if(value > peak)
		{
			peak = value;
			peak_idx = i;
		}
	}
	*start = peak_idx;

	int trough_idx = peak_idx;
	int16_t trough = values[trough_idx];
	for(int i = trough_idx; i < extent; i++)
	{
		if(indices[i] == -1)
			continue;

		int16_t value = values[i];
		if(value < trough)
		{
			trough = value;
			trough_idx = i;
		}
	}
	*end = trough_idx;
}

float CAT_monitor_graph_get_ACH(int start, int end)
{
	if(start == end)
		return -1;
	if(values[start] == values[end])
		return -1;

	int16_t max_ppm = values[start];
	int16_t min_ppm = values[end];
	float decay_concentration = min_ppm + (float) (max_ppm - min_ppm) / (float) M_E;
	
    // Decay time (Assume for simplicity it's between max and baseline times)
   // <A FUNCTION THAT GIVES ME THE TIME that the concentration dropped below decay_concentration and returns decay_time)

	uint64_t decay_time = 0;
	for (int cursor = start; cursor <= end; cursor++)
	{
		if (indices[cursor] == -1)
			continue;

		if(values[cursor] < decay_concentration)
		{
			decay_time = timestamps[cursor];
			break;
		}
	}
	if(decay_time == 0)
		return -1;

    // Calculate ACH
    float ach = 1.0 / ((float) (decay_time - timestamps[start]) / 3600.0);  // Convert to hours
	return ach;
}
