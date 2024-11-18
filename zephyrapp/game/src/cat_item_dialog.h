#pragma once

#include "cat_machine.h"
#include "cat_item.h"

void CAT_anchor_item_dialog(CAT_machine_state state, CAT_item_type type, int* ptr);
void CAT_MS_item_dialog(CAT_machine_signal signal);
void CAT_render_item_dialog();