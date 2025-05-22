#pragma once

#include "cat_machine.h"

typedef struct CAT_menu_node
{
	const char* title;

	void (*proc)(void);
	CAT_machine_state state;

	int selector;
	struct CAT_menu_node* children[];
} CAT_menu_node;

void CAT_MS_menu(CAT_machine_signal signal);
void CAT_render_menu();

void CAT_MS_insights(CAT_machine_signal signal);
void CAT_render_insights();

void CAT_MS_manual(CAT_machine_signal signal);
void CAT_render_manual();

void CAT_MS_magic(CAT_machine_signal signal);
void CAT_render_magic();

void CAT_MS_hedron(CAT_machine_signal signal);
void CAT_render_hedron();

void CAT_MS_debug(CAT_machine_signal signal);
void CAT_render_debug();

void CAT_MS_graph_spoof(CAT_machine_signal signal);
void CAT_render_graph_spoof();

void CAT_MS_colour_picker(CAT_machine_signal signal);
void CAT_render_colour_picker();