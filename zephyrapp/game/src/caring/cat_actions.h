#pragma once

#include "cat_machine.h"
#include "cat_study.h"

void CAT_MS_play(CAT_FSM_signal signal);
void CAT_render_action();

void CAT_MS_laser(CAT_FSM_signal signal);
void CAT_render_laser();

void CAT_MS_feed(CAT_FSM_signal signal);