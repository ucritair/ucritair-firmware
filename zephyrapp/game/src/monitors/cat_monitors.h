#pragma once

#include "cat_machine.h"
#include "cat_core.h"

typedef enum
{
	CAT_MONITOR_PAGE_SUMMARY,
	CAT_MONITOR_PAGE_DETAILS,
	CAT_MONITOR_PAGE_SPARKLINES,
	CAT_MONITOR_PAGE_CALENDAR,
	CAT_MONITOR_PAGE_ACH,
	CAT_MONITOR_PAGE_CLOCK,
	CAT_MONITOR_PAGE_GAMEPLAY,
	CAT_MONITOR_PAGE_COUNT
} CAT_monitor_page;

void CAT_monitor_advance();
void CAT_monitor_retreat();
void CAT_monitor_seek(int target);
int CAT_monitor_tell();
void CAT_monitor_exit();
void CAT_monitor_soft_exit();

void CAT_monitor_colour_bg(uint16_t colour);
void CAT_monitor_colour_fg(uint16_t colour);
uint16_t CAT_monitor_bg_colour();
uint16_t CAT_monitor_fg_colour();

void CAT_monitor_MS_summary(CAT_machine_signal signal);
void CAT_monitor_render_summary();

void CAT_monitor_render_details();

void CAT_monitor_MS_sparklines(CAT_machine_signal signal);
void CAT_monitor_render_sparklines();

void CAT_monitor_MS_calendar(CAT_machine_signal signal);
void CAT_monitor_render_calendar();

void CAT_monitor_MS_ACH(CAT_machine_signal signal);
void CAT_monitor_render_ACH();

void CAT_monitor_MS_clock(CAT_machine_signal signal);
void CAT_monitor_render_clock();

void CAT_monitor_MS_gameplay(CAT_machine_signal signal);
void CAT_monitor_render_gameplay();

void CAT_MS_monitor(CAT_machine_signal signal);