#include "cat_dialogue.h"

#include <stddef.h>
#include "cat_math.h"
#include "cat_render.h"
#include "cat_gui.h"
#include "cat_input.h"

static const CAT_dialogue_node* current = NULL;
static uint8_t line_idx = 0;
static uint8_t edge_idx = 0;
static bool in_dialogue = false;

void CAT_enter_dialogue(const CAT_dialogue_node* node)
{
	current = node;
	line_idx = 0;
	edge_idx = 0;
	in_dialogue = true;
}

void CAT_change_dialogue_response(int dir)
{
	edge_idx = wrap(edge_idx+dir, current->edge_count);
}

void CAT_progress_dialogue()
{
	if(line_idx >= current->line_count-1)
	{
		if(edge_idx < current->edge_count)
		{
			CAT_dialogue_edge* edge = &current->edges[edge_idx];
			if(edge->proc != NULL)
				edge->proc();
			if(edge->node != NULL)
			{
				CAT_enter_dialogue(edge->node);
				return;
			}
		}
		CAT_exit_dialogue();
	}
	else
	{
		line_idx += 1;
	}
}

void CAT_exit_dialogue()
{
	in_dialogue = false;
}

bool CAT_in_dialogue()
{
	return in_dialogue;
}

bool CAT_dialogue_needs_response()
{
	return line_idx == current->line_count-1;
}

const char* CAT_get_dialogue_line()
{
	return current->lines[line_idx];
}

int CAT_get_dialogue_response_count()
{
	return current->edge_count;
}

const char* CAT_get_dialogue_response(int idx)
{
	return current->edges[idx].text;
}

void CAT_dialogue_io()
{
	if(CAT_input_pressed(CAT_BUTTON_B))
		CAT_exit_dialogue();
	
	if(CAT_input_pressed(CAT_BUTTON_A))
		CAT_progress_dialogue();

	if(CAT_dialogue_needs_response())
	{
		if(CAT_input_pressed(CAT_BUTTON_DOWN))
			CAT_change_dialogue_response(1);
		if(CAT_input_pressed(CAT_BUTTON_UP))
			CAT_change_dialogue_response(-1);
	}
}

#define MARGIN_X 4
#define MARGIN_Y 4

void CAT_render_dialogue()
{
	CAT_frameberry(CAT_BLACK);

	int cursor_y = MARGIN_Y;
	CAT_set_text_mask(MARGIN_X, MARGIN_Y, CAT_LCD_SCREEN_W-MARGIN_X, CAT_LCD_SCREEN_H-MARGIN_Y);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	CAT_set_text_colour(CAT_WHITE);
	cursor_y = CAT_draw_textf(MARGIN_X, cursor_y, "%s\n\n", current->lines[line_idx]);

	if(CAT_dialogue_needs_response())
	{
		for(int i = 0; i < CAT_get_dialogue_response_count(); i++)
		{
			CAT_set_text_mask(MARGIN_X, MARGIN_Y, CAT_LCD_SCREEN_W-MARGIN_X, CAT_LCD_SCREEN_H-MARGIN_Y);
			CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
			CAT_set_text_colour(CAT_WHITE);
			const char* fmt = i == edge_idx ? "%s <\n" : "%s\n";
			cursor_y = CAT_draw_textf(MARGIN_X, cursor_y, fmt, current->edges[i].text);
		}
	}
}