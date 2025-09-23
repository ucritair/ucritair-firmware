#pragma once

#include <stdbool.h>

void CAT_effect_start_aperture_blackout(float duration, float direction);
void CAT_effect_cancel_aperture_blackout();
bool CAT_effect_poll_aperture_blackout();

void CAT_effects_tick();
void CAT_effects_render();