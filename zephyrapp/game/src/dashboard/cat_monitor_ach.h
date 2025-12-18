#include "cat_monitors.h"

#include <stdint.h>
#include "cat_time.h"

void CAT_monitor_ACH_set_view(int view);
void CAT_monitor_ACH_set_data(int16_t* values, uint64_t* timestamps, int32_t* indices, int first_idx, int last_idx);
void CAT_monitor_ACH_auto_cursors();
void CAT_monitor_ACH_set_cursors(int start, int end);
void CAT_monitor_ACH_get_cursors(int* start, int* end);
float CAT_monitor_ACH_calculate();

void CAT_monitor_ACH_enter(CAT_datetime date);