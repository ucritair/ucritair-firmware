#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
	const char* text;
	const struct CAT_dialogue_node* node;
	void (*proc) ();
} CAT_dialogue_edge;

typedef struct
{
	const char** lines;
	int8_t line_count;

	const CAT_dialogue_edge* edges;
	int8_t edge_count;
} CAT_dialogue_node;

void CAT_enter_dialogue(const CAT_dialogue_node* node);
void CAT_change_dialogue_response(int dir);
void CAT_progress_dialogue();
void CAT_exit_dialogue();
bool CAT_in_dialogue();

const char* CAT_get_dialogue_line();
bool CAT_dialogue_needs_response();
int CAT_get_dialogue_response_count();
const char* CAT_get_dialogue_response(int idx);

void CAT_dialogue_io();
void CAT_render_dialogue();

typedef struct
{
	struct dialogue_profile_entry
	{
		const CAT_dialogue_node* node;
		uint8_t weight;
	}* entries;
	uint8_t entry_count;
} CAT_dialogue_profile;

void CAT_activate_dialogue_profile(const CAT_dialogue_profile* profile);
const CAT_dialogue_node* CAT_poll_dialogue_profile();