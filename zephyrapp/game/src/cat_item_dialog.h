#pragma once

#include "cat_machine.h"
#include "cat_item.h"

typedef bool (*CAT_item_filter)(int item_id);

void CAT_filter_item_dialog(CAT_item_filter filter);
void CAT_anchor_item_dialog(int* anchor);
void CAT_MS_item_dialog(CAT_machine_signal signal);
void CAT_render_item_dialog();