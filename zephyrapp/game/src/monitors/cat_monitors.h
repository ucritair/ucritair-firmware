#pragma once

#include "cat_machine.h"

void CAT_monitor_advance();
void CAT_monitor_retreat();
void CAT_monitor_exit();

void CAT_monitor_MS_summary(CAT_machine_signal signal);
void CAT_monitor_render_summary();

void CAT_monitor_render_details();

void CAT_monitor_MS_sparklines(CAT_machine_signal signal);
void CAT_monitor_render_sparklines();

void CAT_monitor_MS_calendar(CAT_machine_signal signal);
void CAT_monitor_render_calendar();

void CAT_monitor_MS_graph(CAT_machine_signal signal);
void CAT_monitor_render_graph();

void CAT_monitor_MS_gameplay(CAT_machine_signal signal);
void CAT_monitor_render_gameplay();

void CAT_monitor_MS_clock(CAT_machine_signal signal);
void CAT_monitor_render_clock();

void CAT_MS_monitor(CAT_machine_signal signal);