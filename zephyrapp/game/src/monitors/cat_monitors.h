#pragma once

#include "cat_machine.h"
#include "cat_core.h"

#define CAT_MONITOR_BG_BLUE RGB8882565(35, 157, 235)

void CAT_monitor_advance();
void CAT_monitor_retreat();
void CAT_monitor_exit();

void CAT_monitor_set_background(uint16_t colour);

void CAT_monitor_MS_summary(CAT_machine_signal signal);
void CAT_monitor_render_summary();

void CAT_monitor_render_details();

void CAT_monitor_MS_sparklines(CAT_machine_signal signal);
void CAT_monitor_render_sparklines();

void CAT_monitor_MS_calendar(CAT_machine_signal signal);
void CAT_monitor_render_calendar();

void CAT_monitor_MS_logs(CAT_machine_signal signal);
void CAT_monitor_render_logs();

void CAT_monitor_MS_clock(CAT_machine_signal signal);
void CAT_monitor_render_clock();

void CAT_monitor_MS_gameplay(CAT_machine_signal signal);
void CAT_monitor_render_gameplay();

void CAT_MS_monitor(CAT_machine_signal signal);