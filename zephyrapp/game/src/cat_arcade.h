#pragma once

#include "cat_machine.h"

//////////////////////////////////////////////////////////////////////////
// MACHINE

void CAT_MS_arcade(CAT_machine_signal signal);
void CAT_render_arcade();


//////////////////////////////////////////////////////////////////////////
// SNAKE

extern int snake_high_score;

void CAT_MS_snake(CAT_machine_signal signal);
void CAT_render_snake();
