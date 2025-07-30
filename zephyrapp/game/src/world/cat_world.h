#pragma once

#include <stdint.h>
#include "cat_core.h"
#include "cat_machine.h"
#include "cat_render.h"


//////////////////////////////////////////////////////////////////////////
// MOVEMENT

void CAT_world_get_position(int* x, int* y);


//////////////////////////////////////////////////////////////////////////
// WORLD

void CAT_MS_world(CAT_machine_signal signal);
void CAT_render_world();

