#pragma once

#include "cat_machine.h"

void CAT_MS_study(CAT_machine_signal signal);
void CAT_MS_play(CAT_machine_signal signal);
void CAT_render_action();

void CAT_MS_laser(CAT_machine_signal signal);
void CAT_render_laser();

void CAT_MS_feed(CAT_machine_signal signal);
void CAT_render_feed();