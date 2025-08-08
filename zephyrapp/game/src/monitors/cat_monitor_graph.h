#pragma once

#include <stdint.h>
#include "cat_core.h"

typedef enum
{
	CAT_MONITOR_GRAPH_VIEW_CO2,
	CAT_MONITOR_GRAPH_VIEW_PN_10_0,
	CAT_MONITOR_GRAPH_VIEW_PM_2_5,
	CAT_MONITOR_GRAPH_VIEW_TEMP,
	CAT_MONITOR_GRAPH_VIEW_RH,
	CAT_MONITOR_GRAPH_VIEW_PRESS,
	CAT_MONITOR_GRAPH_VIEW_VOC,
	CAT_MONITOR_GRAPH_VIEW_NOX,
	CAT_MONITOR_GRAPH_VIEW_MAX
} CAT_monitor_graph_view;

void CAT_monitor_graph_set_view(int view);
void CAT_monitor_graph_set_sample_count(int sample_count);

void CAT_monitor_graph_load_date(CAT_datetime date);
bool CAT_monitor_graph_is_loading();
bool CAT_monitor_graph_did_load_succeed();

int16_t* CAT_monitor_graph_get_values();
uint64_t* CAT_monitor_graph_get_timestamps();
int32_t* CAT_monitor_graph_get_indices();
int CAT_monitor_graph_get_extent();

void CAT_monitor_graph_tick();

void CAT_monitor_graph_set_focus(int idx);
void CAT_monitor_graph_set_scale(int scale);
void CAT_monitor_graph_draw(int x, int y, int h);
void CAT_monitor_graph_draw_reticle(int x, uint16_t c);
void CAT_monitor_graph_draw_cursor(int x, uint16_t c);

void CAT_monitor_graph_enter(CAT_datetime date);
void CAT_monitor_graph_logic();
bool CAT_monitor_graph_should_exit();
void CAT_monitor_graph_render();
