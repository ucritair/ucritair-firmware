#pragma once

#include "cat_machine.h"

typedef struct CAT_menu_node
{
	const char* title;

	void (*proc)(void);
	CAT_FSM_state state;

	int selector;
	struct CAT_menu_node* children[];
} CAT_menu_node;

void CAT_MS_menu(CAT_FSM_signal signal);
void CAT_render_menu();

void CAT_MS_insights(CAT_FSM_signal signal);
void CAT_render_insights();

void CAT_MS_manual(CAT_FSM_signal signal);
void CAT_render_manual();

void CAT_MS_magic(CAT_FSM_signal signal);
void CAT_render_magic();

void CAT_MS_hedron(CAT_FSM_signal signal);
void CAT_render_hedron();

void CAT_MS_debug(CAT_FSM_signal signal);
void CAT_render_debug();

void CAT_MS_colour_picker(CAT_FSM_signal signal);
void CAT_render_colour_picker();

void CAT_MS_palette_picker(CAT_FSM_signal signal);
void CAT_render_palette_picker();