#pragma once

#include "cat_machine.h"
#include "cat_item.h"

extern CAT_item_list bag;
extern int coins;

void CAT_MS_bag(CAT_machine_signal signal);
void CAT_render_bag();