#pragma once

#include "cat_machine.h"

void CAT_MS_menu(CAT_FSM_signal signal);
void CAT_render_menu();

void CAT_MS_insights(CAT_FSM_signal signal);
void CAT_render_insights();

void CAT_MS_manual(CAT_FSM_signal signal);
void CAT_render_manual();

void CAT_MS_debug(CAT_FSM_signal signal);
void CAT_render_debug();

void CAT_MS_colour_picker(CAT_FSM_signal signal);
void CAT_render_colour_picker();