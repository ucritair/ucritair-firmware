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
	return current->edge_count > 0 && line_idx == current->line_count-1;
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

#define BOX_X 0
#define BOX_W CAT_LCD_SCREEN_W
#define BOX_H (CAT_LCD_SCREEN_H / 4)
#define BOX_Y (CAT_LCD_SCREEN_H - BOX_H)
#define BOX_MARGIN 8

void CAT_render_dialogue()
{
	CAT_fillberry(BOX_X, BOX_Y, BOX_W, BOX_H, CAT_WHITE);
	CAT_strokeberry(BOX_X, BOX_Y, BOX_W, BOX_H, CAT_BLACK);

	int cursor_y = BOX_Y + BOX_MARGIN;
	CAT_set_text_mask(BOX_X+BOX_MARGIN, -1, BOX_X+BOX_W-BOX_MARGIN, -1);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	cursor_y = CAT_draw_textf(BOX_X + BOX_MARGIN, cursor_y, "%s\n", current->lines[line_idx]);

	if(CAT_dialogue_needs_response())
	{
		cursor_y += 4;
		CAT_lineberry(BOX_X + BOX_MARGIN, cursor_y, BOX_X+BOX_W-BOX_MARGIN, cursor_y, CAT_GREY);
		cursor_y += 8;

		for(int i = 0; i < CAT_get_dialogue_response_count(); i++)
		{
			const char* fmt = i == edge_idx ? "%s <\n" : "%s\n";
			cursor_y = CAT_draw_textf(BOX_X+BOX_MARGIN, cursor_y, fmt, current->edges[i].text);
		}
	}
}

static const CAT_dialogue_profile* active_profile;

static int entry_indices_backing[64];
static CAT_int_list entry_indices;
static int weight_max;

static bool deliver_opener = false;

int weight_cmp(int i, int j)
{
	return sgn(active_profile->entries[i].weight - active_profile->entries[j].weight);
}

void CAT_activate_dialogue_profile(CAT_dialogue_profile* profile)
{
	active_profile = profile;
	
	CAT_ilist(&entry_indices, entry_indices_backing, 64);
	weight_max = -1;

	for(int i = 0; i < active_profile->entry_count; i++)
	{
		CAT_dialogue_profile_entry* entry = &active_profile->entries[i];
		if(entry->is_active_proc == NULL || entry->is_active_proc())
		{
			CAT_ilist_push(&entry_indices, i);
			weight_max = max(weight_max, entry->weight);
		}
	}
	CAT_ilist_shuffle(&entry_indices);
	CAT_ilist_sort_by_proc(&entry_indices, weight_cmp);

	deliver_opener = CAT_rand_float(0, 1) < active_profile->opener_probability;
}

CAT_dialogue_node* pick_entry()
{
	int weight = CAT_rand_int(0, weight_max);
	for(int i = 0; i < entry_indices.length; i++)
	{
		int idx = entry_indices.data[i];
		CAT_dialogue_profile_entry* entry = &active_profile->entries[idx];
		if(weight <= entry->weight)
			return active_profile->entries[idx].node;
	}
	return NULL;
}

const CAT_dialogue_node* CAT_poll_dialogue_profile()
{
	if(active_profile->mandatory_node != NULL)
	{
		if(deliver_opener)
		{
			deliver_opener = false;
			CAT_dialogue_node* opener = pick_entry(active_profile);
			if(opener != NULL)
				return opener;
		}
		return active_profile->mandatory_node;
	}
	else
	{
		return pick_entry(active_profile);
	}

	return NULL;
}