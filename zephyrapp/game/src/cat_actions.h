#pragma once

#include "cat_machine.h"
#include "cat_math.h"
#include "cat_render.h"
#include "cat_item.h"
#include "cat_item_dialog.h"

void CAT_MS_feed(CAT_machine_signal signal);
void CAT_MS_study(CAT_machine_signal signal);
void CAT_MS_play(CAT_machine_signal signal);
void CAT_MS_laser(CAT_machine_signal signal);

void CAT_render_action();
void CAT_render_laser();