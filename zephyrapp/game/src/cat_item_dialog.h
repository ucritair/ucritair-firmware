#pragma once

#include "cat_machine.h"
#include "cat_item.h"

void CAT_filter_item_dialog(CAT_item_filter filter);
void CAT_target_item_dialog(int* anchor, bool enforce_valid);
void CAT_MS_item_dialog(CAT_machine_signal signal);
void CAT_render_item_dialog();