#pragma once

#include "cat_machine.h"

void CAT_MS_arcade(CAT_FSM_signal signal);
void CAT_render_arcade();

extern int snake_highscore;
void CAT_MS_snake(CAT_FSM_signal signal);
void CAT_render_snake();

extern int mines_highscore;
void CAT_MS_mines(CAT_FSM_signal signal);
void CAT_render_mines();

extern int foursquares_highscore;
void CAT_MS_foursquares(CAT_FSM_signal signal);
void CAT_render_foursquares();

extern int stroop_highscore;
void CAT_MS_stroop(CAT_FSM_signal signal);
void CAT_render_stroop();