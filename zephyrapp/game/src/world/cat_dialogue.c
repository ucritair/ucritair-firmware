#include "cat_dialogue.h"

#include <stddef.h>
#include "cat_math.h"
#include "cat_render.h"
#include "cat_gui.h"
#include "cat_input.h"

static const CAT_dialogue_node* current = NULL;
static uint8_t line_idx = 0;
static uint8_t edge_idx = 0;

void CAT_enter_dialogue(const CAT_dialogue_node* node)
{
	current = node;
	line_idx = 0;
	edge_idx = 0;
}

void CAT_change_dialogue_response(int dir)
{
	if(current == NULL)
		return;

	edge_idx = wrap(edge_idx+dir, current->edge_count);
}

void CAT_progress_dialogue()
{
	if(current == NULL)
		return;

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
	current = NULL;
}

bool CAT_in_dialogue()
{
	return current != NULL;
}

bool CAT_dialogue_needs_response()
{
	if(current == NULL)
		return false;

	return current->edge_count > 0 && line_idx == current->line_count-1;
}

const char* CAT_get_dialogue_line()
{
	if(current == NULL)
		return "";

	return current->lines[line_idx];
}

int CAT_get_dialogue_response_count()
{
	if(current == NULL)
		return 0;

	return current->edge_count;
}

const char* CAT_get_dialogue_response(int idx)
{
	if(current == NULL)
		return "";

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
	if(current == NULL)
		return;

	int cursor_y = BOX_Y + BOX_MARGIN;
	int max_response_len = 0;
	if(CAT_dialogue_needs_response())
	{
		for(int i = 0; i < CAT_get_dialogue_response_count(); i++)
		{
			max_response_len = max(max_response_len, strlen(CAT_get_dialogue_response(i)));
		}
	}
	int text_start_x = BOX_X + BOX_MARGIN;
	int response_start_x = BOX_X + BOX_W - ((max_response_len + 2) * CAT_GLYPH_WIDTH) - BOX_MARGIN;

	CAT_fillberry(BOX_X, BOX_Y, BOX_W, BOX_H, CAT_BLACK);

	CAT_set_text_mask(text_start_x, -1, response_start_x-BOX_MARGIN, -1);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	CAT_set_text_colour(CAT_WHITE);
	CAT_draw_textf(text_start_x, cursor_y, "%s\n", current->lines[line_idx]);

	if(CAT_dialogue_needs_response())
	{
		CAT_lineberry(response_start_x-BOX_MARGIN, BOX_Y + BOX_MARGIN, response_start_x-BOX_MARGIN, BOX_Y + BOX_H - BOX_MARGIN, CAT_WHITE);

		for(int i = 0; i < CAT_get_dialogue_response_count(); i++)
		{
			const char* fmt = i == edge_idx ? "%s <\n" : "%s\n";
			CAT_set_text_colour(CAT_WHITE);
			cursor_y = CAT_draw_textf(response_start_x, cursor_y, fmt, current->edges[i].text);
		}
	}
}

static const CAT_dialogue_profile* active_profile = NULL;

static int entry_indices_backing[64];
static CAT_int_list entry_indices;
static int weight_max;

static bool deliver_opener = false;

int weight_cmp(int i, int j)
{
	return sgn(active_profile->entries[i].weight - active_profile->entries[j].weight);
}

void CAT_activate_dialogue_profile(const CAT_dialogue_profile* profile)
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
	if(active_profile != NULL)
	{
		int weight = CAT_rand_int(0, weight_max);
		for(int i = 0; i < entry_indices.length; i++)
		{
			int idx = entry_indices.data[i];
			CAT_dialogue_profile_entry* entry = &active_profile->entries[idx];
			if(weight >= entry->weight)
				return active_profile->entries[idx].node;
		}
	}

	return NULL;
}

const CAT_dialogue_node* CAT_poll_dialogue_profile()
{
	if(active_profile != NULL)
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
	}

	return NULL;
}