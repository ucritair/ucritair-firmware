#pragma once

#include <stdint.h>
#include "cat_item.h"
#include <stdbool.h>

typedef struct
{
	CAT_item_bundle inputs[9];
	int output;

	bool shapeless;

	bool known;
	bool mastered;
} CAT_recipe;

void CAT_MS_crafting(CAT_machine_signal signal);
void CAT_render_crafting();