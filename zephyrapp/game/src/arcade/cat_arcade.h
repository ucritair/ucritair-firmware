#pragma once

#include "cat_machine.h"

void CAT_MS_arcade(CAT_FSM_signal signal);
void CAT_render_arcade();

extern int snake_high_score;
void CAT_MS_snake(CAT_FSM_signal signal);
void CAT_render_snake();

void CAT_MS_mines(CAT_FSM_signal signal);
void CAT_render_mines();

void CAT_MS_foursquares(CAT_FSM_signal signal);
void CAT_render_foursquares();