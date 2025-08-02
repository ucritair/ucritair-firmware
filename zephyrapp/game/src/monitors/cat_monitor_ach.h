#include <stdint.h>

void CAT_monitor_graph_set_ACH_data(int view, int16_t* values, uint64_t* timestamps, int32_t* indices, int extent);
void CAT_monitor_graph_auto_ACH_cursors(int* start, int* end);
float CAT_monitor_graph_get_ACH(int start, int end);