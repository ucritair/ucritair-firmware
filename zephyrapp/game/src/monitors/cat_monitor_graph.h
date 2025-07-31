#pragma once

#include <stdint.h>
#include "cat_core.h"

void CAT_monitor_graph_load_date(CAT_datetime date);

void CAT_monitor_graph_logic();
bool CAT_monitor_graph_is_loading();
bool CAT_monitor_graph_should_exit();

void CAT_monitor_graph_render();
