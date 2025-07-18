#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
	const char* text;

	enum
	{
		CAT_DIALOGUE_EDGE_TYPE_NODE,
		CAT_DIALOGUE_EDGE_TYPE_PROC
	} type;

	union
	{
		struct CAT_dialogue_node* node;
		void (*proc) (const char*);
	};
} CAT_dialogue_edge;

typedef struct
{
	const char** lines;
	int8_t line_count;

	CAT_dialogue_edge* edges;
	int8_t edge_count;
} CAT_dialogue_node;

void CAT_enter_dialogue(CAT_dialogue_node* node);
void CAT_change_dialogue_response();
void CAT_progress_dialogue();
void CAT_exit_dialogue();
bool CAT_in_dialogue();

const char* CAT_get_dialogue_line();
bool CAT_dialogue_needs_response();
int CAT_get_dialogue_response_count();
const char* CAT_get_dialogue_response(int idx);

void CAT_dialogue_io();
void CAT_render_dialogue();