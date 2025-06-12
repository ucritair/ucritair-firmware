#pragma once

#include "cat_machine.h"
#include "cat_item.h"

extern int coins;

void CAT_bag_clear();
void CAT_bag_add(int item_id, int count);
void CAT_bag_remove(int item_id, int count);

void CAT_MS_inventory(CAT_machine_signal signal);
void CAT_render_inventory();

void CAT_MS_inspector(CAT_machine_signal signal);
void CAT_render_inspector();