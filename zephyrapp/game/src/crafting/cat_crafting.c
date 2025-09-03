#include "cat_crafting.h"

#include "cat_input.h"
#include "cat_gui.h"
#include "cat_render.h"
#include "cat_gizmos.h"

bool incl_excl_pass(const CAT_recipe* recipe, CAT_item_bundle* inputs)
{
	int recipe_count = 0;
	for(int i = 0; i < 9; i++)
		recipe_count += recipe->inputs[i].item != -1 ? 1 : 0;
	int input_count = 0;
	for(int i = 0; i < 9; i++)
		input_count += inputs[i].item != -1 ? 1 : 0;
	if(input_count != recipe_count)
		return false;

	for(int i = 0; i < 9; i++)
	{
		CAT_item_bundle* a = &recipe->inputs[i];
		for(int j = 0; j < 9; j++)
		{
			CAT_item_bundle* b = &inputs[j];
			if(b->item != a->item)
				return false;
		}
	}

	return true;
}

bool count_pass(const CAT_recipe* recipe, CAT_item_bundle* inputs)
{
	for(int i = 0; i < 9; i++)
	{
		CAT_item_bundle* a = &recipe->inputs[i];
		for(int j = 0; j < 9; j++)
		{
			CAT_item_bundle* b = &inputs[j];
			if(b->item == a->item && b->count < a->count)
				return false;
		}
	}
	return false;
}

void get_bounding_box
(
	CAT_item_bundle* inputs,
	int* r0_out, int* c0_out, int* r1_out, int* c1_out
)
{
	*r0_out = -1;
	*c0_out = -1;
	*r1_out = -1;
	*c1_out = -1;

	for(int r = 0; r < 3; r++)
	{
		for(int c = 0; c < 3; c++)
		{
			int i = r * 3 + c;
			CAT_item_bundle* a = &inputs[i];
			if(a->item != -1)
			{
				*r0_out = min(*r0_out, r);
				*c0_out = min(*c0_out, c);
				*r1_out = max(*r1_out, r);
				*c1_out = max(*c1_out, c);
			}
		}
	}
}

bool shape_pass(const CAT_recipe* recipe, CAT_item_bundle* inputs)
{
	int r0_r, c0_r, r1_r, c1_r;
	get_bounding_box(recipe->inputs, &r0_r, &c0_r, &r1_r, &c1_r);
	if(r0_r == -1 || c0_r == -1 || r1_r == -1 || c1_r == -1)
		return false;
	int r0_i, c0_i, r1_i, c1_i;
	get_bounding_box(inputs, &r0_i, &c0_i, &r1_i, &c1_i);
	if(r0_i == -1 || c0_i == -1 || r1_i == -1 || c1_i == -1)
		return false;

	int h_r = r1_r-r0_r+1;
	int h_i = r1_i-r0_i+1;
	if(h_i != h_r)
		return false;
	int w_r = c1_r-c0_r+1;
	int w_i = c1_i-c1_i+1;
	if(w_i != w_r)
		return false;

	for(int dr = 0; dr < h_r; dr++)
	{
		for(int dc = 0; dc < w_r; dc++)
		{
			int i_r = (r0_r+dr) * 3 + (c0_r+dc);
			int i_i = (r0_i+dr) * 3 + (c0_i+dc);
			CAT_item_bundle* a = &recipe->inputs[i_r];
			CAT_item_bundle* b = &inputs[i_i];
			if(b->item != a->item)
				return false;
		}
	}

	return false;
}

bool recipe_validate(const CAT_recipe* recipe, CAT_item_bundle* inputs)
{
	if(!incl_excl_pass(recipe, inputs))
		return false;
	if(!count_pass(recipe, inputs))
		return false;
	if(!recipe->shapeless)
	{
		if(!shape_pass(recipe, inputs))
			return false;
	}
	return true;
}

static CAT_item_bundle inputs[9];

int get_total(int item_id)
{
	int total = 0;
	for(int i = 0; i < 9; i++)
		total += inputs[i].item == item_id ? inputs[i].count : 0;
	return total;
}

void increase_count(int idx)
{
	int item = inputs[idx].item;
	int total = get_total(item);
	if(inputs[idx].count < 99 && total < item_table.counts[item])
		inputs[idx].count += 1;
}

void decrease_count(int idx)
{
	if(inputs[idx].count > 1)
		inputs[idx].count -= 1;
	else
	{
		inputs[idx].item = -1;
		inputs[idx].count = 0;
	}
}

static int selector = 0;
static bool selecting = false;

void select_proc(int item_id)
{
	inputs[selector] = (CAT_item_bundle)
	{
		.item = item_id,
		.count = 1
	};
	selecting = false;
}

void CAT_MS_crafting(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			CAT_set_render_callback(CAT_render_crafting);
			CAT_gui_begin_item_grid_context(false);

			for(int i = 0; i < 9; i++)
			{
				inputs[i] = (CAT_item_bundle)
				{
					.item = -1,
					.count = 0
				};
			}
		}
		break;

		case CAT_MACHINE_SIGNAL_TICK:
		{	
			if(selecting)
			{
				CAT_gui_begin_item_grid();
				CAT_gui_item_grid_add_tab("Items", NULL, select_proc);		

				for(int i = 0; i < item_table.length; i++)
				{
					if(get_total(i) >= item_table.counts[i])
						continue;
					CAT_gui_item_grid_cell(i);
				}

				if(CAT_input_dismissal())
					selecting = false;
			}
			else
			{
				if(CAT_input_pressed(CAT_BUTTON_UP))
					selector -= 3;
				if(CAT_input_pressed(CAT_BUTTON_DOWN))
					selector += 3;
				if(CAT_input_pressed(CAT_BUTTON_LEFT))
					selector -= 1;
				if(CAT_input_pressed(CAT_BUTTON_RIGHT))
					selector += 1;
				selector = clamp(selector, 0, 8);

				if(CAT_input_pressed(CAT_BUTTON_A))
				{
					if(inputs[selector].item != -1)
						increase_count(selector);
					else
						selecting = true;
				}
				if(CAT_input_pressed(CAT_BUTTON_B))
				{
					if(inputs[selector].item != -1)
						decrease_count(selector);
					else
						CAT_machine_back();
				}
			}
		}
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
		{

		}
		break;
	}
}

#define CELL_SIZE 48
#define GRID_ROWS 3
#define GRID_COLS 3
#define GRID_H (CELL_SIZE * GRID_ROWS)
#define GRID_W (CELL_SIZE * GRID_COLS)
#define CENTER_X (CAT_LCD_SCREEN_W/2)
#define CENTER_Y (CAT_LCD_SCREEN_H/2)
#define GRID_X (CENTER_X-(GRID_W/2))
#define GRID_Y (CENTER_Y-(GRID_H/2))

#define OUTPUT_X (CENTER_X-(CELL_SIZE/2))
#define OUTPUT_Y (GRID_Y - (CELL_SIZE + 16))

void CAT_render_crafting()
{
	CAT_frameberry(CAT_BLACK);

	int y = GRID_Y-1;
	for(int r = 0; r < GRID_ROWS; r++)
	{
		int x = GRID_X-1;
		for(int c = 0; c < GRID_COLS; c++)
		{
			CAT_draw_corner_box(x, y, x+CELL_SIZE, y+CELL_SIZE, CAT_GREY);

			int i = r * 3 + c;
			CAT_item* item = CAT_get_item(inputs[i].item);
			if(item != NULL)
			{
				CAT_set_sprite_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y);
				CAT_draw_sprite(item->sprite, 0, x+CELL_SIZE/2, y+CELL_SIZE/2);
				
				int count = inputs[i].count;
				int width = CAT_GLYPH_WIDTH * ((count > 9) ? 2 : 1);
				CAT_set_text_colour(CAT_WHITE);
				CAT_draw_textf(x + CELL_SIZE-width-2, y+CELL_SIZE-CAT_GLYPH_HEIGHT-3, "%d", count);
			}

			x += CELL_SIZE;
		}
		y += CELL_SIZE;
	}

	int r = selector / 3;
	int c = selector % 3;
	CAT_strokeberry
	(
		GRID_X + c * CELL_SIZE-1, GRID_Y + r * CELL_SIZE-1,
		CELL_SIZE+1, CELL_SIZE+1,
		CAT_WHITE
	);

	CAT_strokeberry(OUTPUT_X, OUTPUT_Y, CELL_SIZE, CELL_SIZE, CAT_GREY);
}