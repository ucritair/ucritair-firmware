#pragma once

#include "cat_machine.h"

typedef struct CAT_menu_node
{
	const char* title;

	void (*proc)(void);
	CAT_machine_state state;

	struct CAT_menu_node* children[];
} CAT_menu_node;

void CAT_MS_menu(CAT_machine_signal signal);
void CAT_render_menu();

void CAT_MS_hedron(CAT_machine_signal signal);
void CAT_render_hedron();

void CAT_MS_magic(CAT_machine_signal signal);
void CAT_render_magic();

void CAT_MS_debug(CAT_machine_signal signal);
void CAT_render_debug();

void CAT_MS_cheats(CAT_machine_signal signal);
void CAT_render_cheats();