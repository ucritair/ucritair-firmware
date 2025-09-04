#pragma once

#include <stdint.h>
#include "cat_item.h"
#include <stdbool.h>

typedef enum
{
	CAT_RECIPE_FLAG_NONE = 0,
	CAT_RECIPE_FLAG_SHAPELESS = (1 << 0),
	CAT_RECIPE_FLAG_KNOWN = (1 << 1),
	CAT_RECIPE_FLAG_MASTERED = (1 << 2)
} CAT_recipe_flag;

typedef struct
{
	int inputs[9];
	int output;
	uint16_t flags;
} CAT_recipe;

void CAT_MS_crafting(CAT_machine_signal signal);
void CAT_render_crafting();