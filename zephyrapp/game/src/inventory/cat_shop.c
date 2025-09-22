#include "cat_item.h"

#include "cat_gui.h"
#include "cat_input.h"
#include "cat_render.h"
#include "item_assets.h"
#include "cat_curves.h"
#include "cat_gizmos.h"

#define INSPECTOR_BG_COLOUR CAT_RGB8882565(142, 171, 174)
#define INSPECTOR_MARGIN 8

static bool buy_filter(int item_id)
{
	if(item_id < 0 || item_id >= item_table.length)
		return false;
	return
	item_table.data[item_id].can_buy &&
	item_table.data[item_id].price > 0;
}
static bool sell_filter(int item_id)
{
	if(item_id < 0 || item_id >= item_table.length)
		return false;
	return
	item_table.data[item_id].can_sell &&
	item_table.counts[item_id] > 0;
}
static bool prop_filter(int item_id)
{
	return buy_filter(item_id) &&
	item_table.data[item_id].type == CAT_ITEM_TYPE_PROP;
}
static bool tool_filter(int item_id)
{
	return buy_filter(item_id) &&
	item_table.data[item_id].type == CAT_ITEM_TYPE_TOOL &&
	!(
		item_table.data[item_id].type == CAT_ITEM_TYPE_TOOL &&
		item_table.data[item_id].tool_type != CAT_TOOL_TYPE_FOOD &&
		item_table.counts[item_id] > 0
	);
}
static bool key_filter(int item_id)
{
	return buy_filter(item_id) &&
	item_table.data[item_id].type == CAT_ITEM_TYPE_KEY;
}

static struct
{
	const char* title;
	CAT_item_filter filter;
} tabs[] =
{
	{"ALL", buy_filter},
	{"DECORATIONS", prop_filter},
	{"CARE ITEMS", tool_filter},
	{"KEY ITEMS", key_filter},
	{"SELL", sell_filter}
};
#define NUM_TABS (sizeof(tabs)/sizeof(tabs[0]))

static int checkout_id = -1;
static int purchase_qty = 1;

static float purchase_progress = 0;
static bool purchase_lock = false;

void CAT_bind_checkout(int item_id)
{
	checkout_id = item_id;
	purchase_qty = 1;

	purchase_progress = 0;
	purchase_lock = false;
}

void CAT_MS_checkout(CAT_FSM_signal signal)
{
	switch(signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		{
			CAT_set_render_callback(CAT_render_checkout);
			break;
		}
		case CAT_FSM_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_pushdown_pop();

			if(CAT_input_pulse(CAT_BUTTON_LEFT))
				purchase_qty -= 1;
			if(CAT_input_pulse(CAT_BUTTON_RIGHT))
				purchase_qty += 1;
			
			int purchasable = CAT_inventory_count(coin_item) / item_table.data[checkout_id].price;
			if(purchasable <= 0)
				purchase_qty = 0;
			else
			{
				int max_qty = min(purchasable, 99);
				max_qty = min(max_qty, UINT16_MAX-item_table.counts[checkout_id]);
				purchase_qty = clamp(purchase_qty, 1, max_qty);
			}

			if(CAT_input_held(CAT_BUTTON_A, 0) && purchase_qty > 0)
			{
				if(CAT_input_pressed(CAT_BUTTON_A))
					purchase_progress += 0.1f;

				purchase_progress += CAT_get_delta_time_s();	
				purchase_progress = clamp(purchase_progress, 0, 1);

				if(!purchase_lock)
				{	
					if(purchase_progress >= 0.9f)
						purchase_lock = true;
				}
			}
			else if(CAT_input_released(CAT_BUTTON_A))
			{
				if(purchase_lock)
				{
					CAT_inventory_add(checkout_id, purchase_qty);
					CAT_inventory_remove(coin_item, item_table.data[checkout_id].price * purchase_qty);
				}

				purchase_progress = 0;
				purchase_lock = false;	
			}

			break;
		}
		case CAT_FSM_SIGNAL_EXIT:
		{
			break;
		}
	}
}

#define CHECKOUT_GOLD_COLOUR 0xf606

void CAT_render_checkout()
{
	CAT_item* item = CAT_get_item(checkout_id);

	CAT_frameberry(INSPECTOR_BG_COLOUR);

	if (item == NULL)
		return;

	int cursor_y = INSPECTOR_MARGIN;
	CAT_set_text_scale(2);
	CAT_set_text_colour(CAT_WHITE);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	CAT_set_text_mask(INSPECTOR_MARGIN, -1, CAT_LCD_SCREEN_W-INSPECTOR_MARGIN, -1);
	cursor_y = CAT_draw_textf(INSPECTOR_MARGIN, cursor_y, "%s\n", item->name) + 2;
	CAT_set_text_colour(CAT_WHITE);
	cursor_y = CAT_draw_textf(INSPECTOR_MARGIN, cursor_y, "$%d / x%d owned", item->price, item_table.counts[checkout_id]);
	CAT_set_text_colour(CHECKOUT_GOLD_COLOUR);
	cursor_y = CAT_draw_textf(CAT_LCD_SCREEN_W-INSPECTOR_MARGIN-CAT_GLYPH_WIDTH*7, cursor_y, "$%.6d\n", CAT_inventory_count(coin_item));
	cursor_y += 6;

	int box_h = CAT_LCD_SCREEN_H-INSPECTOR_MARGIN-84-cursor_y;
	CAT_set_sprite_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y);
	for(int scale = 1; scale <= 6; scale++)
	{
		if
		(
			item->sprite->width * scale <= 240 &&
			item->sprite->height * scale <= box_h
		)
			CAT_set_sprite_scale(scale);
	}
	CAT_set_sprite_mask(INSPECTOR_MARGIN, cursor_y, CAT_LCD_SCREEN_W-INSPECTOR_MARGIN, cursor_y + box_h);
	CAT_draw_sprite(item->sprite, 0, 120, cursor_y + box_h/2);
	CAT_strokeberry(INSPECTOR_MARGIN, cursor_y, CAT_LCD_SCREEN_W-INSPECTOR_MARGIN * 2, box_h, CAT_WHITE);
	cursor_y += box_h;
	if(!purchase_lock || CAT_pulse(0.25f))
	{
		CAT_set_text_colour(purchase_lock ? CHECKOUT_GOLD_COLOUR: CAT_WHITE);
		CAT_draw_textf(INSPECTOR_MARGIN, cursor_y + 2, "BUYING %d", purchase_qty);
	}

	cursor_y += 32;

	if(purchase_qty > 0)
	{
		uint16_t colour = purchase_lock ? CHECKOUT_GOLD_COLOUR : CAT_WHITE;

		CAT_set_text_scale(2);
		CAT_set_text_colour(colour);
		CAT_set_text_flags(CAT_TEXT_FLAG_CENTER);
		cursor_y = CAT_draw_textf(120, cursor_y, "%d", purchase_qty);

		int size = 24;
		float progress = CAT_ease_out_quart(purchase_progress);
		int dist = 24 + progress * 40;
		CAT_draw_arrows(120, cursor_y + 12, size, dist, colour);
	}
	else
	{
		CAT_set_text_scale(2);
		CAT_set_text_colour(CAT_WHITE);
		CAT_set_text_flags(CAT_TEXT_FLAG_CENTER | CAT_TEXT_FLAG_WRAP);
		cursor_y = CAT_draw_text(120, cursor_y-8, "Insufficient funds!");
	}
}

static int sale_id = -1;
static int sale_qty = 1;

static float sale_progress = 0;
static bool sale_lock = false;

void CAT_bind_sale(int item_id)
{
	sale_id = item_id;
	sale_qty = 1;

	sale_progress = 0;
	sale_lock = false;
}

void CAT_MS_sale(CAT_FSM_signal signal)
{
	switch(signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		{
			CAT_set_render_callback(CAT_render_sale);
			break;
		}
		case CAT_FSM_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_pushdown_pop();

			if(CAT_inventory_count(sale_id) > 0)
			{
				if(CAT_input_pulse(CAT_BUTTON_LEFT))
					sale_qty -= 1;
				if(CAT_input_pulse(CAT_BUTTON_RIGHT))
					sale_qty += 1;
				sale_qty = clamp(sale_qty, 1, CAT_inventory_count(sale_id));
			}
			else
			{
				sale_qty = 0;
			}

			if(CAT_input_held(CAT_BUTTON_A, 0))
			{
				if(CAT_input_pressed(CAT_BUTTON_A))
					sale_progress += 0.1f;

				sale_progress += CAT_get_delta_time_s();	
				sale_progress = clamp(sale_progress, 0, 1);

				if(!sale_lock)
				{	
					if(sale_progress >= 0.9f)
						sale_lock = true;
				}
			}
			else if(CAT_input_released(CAT_BUTTON_A))
			{
				if(sale_lock)
				{
					CAT_inventory_remove(sale_id, sale_qty);
					CAT_inventory_add(coin_item, item_table.data[sale_id].price * sale_qty);
				}

				sale_progress = 0;
				sale_lock = false;	
			}

			break;
		}
		case CAT_FSM_SIGNAL_EXIT:
		{
			break;
		}
	}
}

void CAT_render_sale()
{
	CAT_item* item = CAT_get_item(sale_id);

	CAT_frameberry(INSPECTOR_BG_COLOUR);

	if (item == NULL)
		return;

	int cursor_y = INSPECTOR_MARGIN;
	CAT_set_text_scale(2);
	CAT_set_text_colour(CAT_WHITE);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	CAT_set_text_mask(INSPECTOR_MARGIN, -1, CAT_LCD_SCREEN_W-INSPECTOR_MARGIN, -1);
	cursor_y = CAT_draw_textf(INSPECTOR_MARGIN, cursor_y, "%s\n", item->name) + 2;
	CAT_set_text_colour(CAT_WHITE);
	cursor_y = CAT_draw_textf(INSPECTOR_MARGIN, cursor_y, "$%d / x%d owned", item->price, item_table.counts[sale_id]);
	CAT_set_text_colour(CHECKOUT_GOLD_COLOUR);
	cursor_y = CAT_draw_textf(CAT_LCD_SCREEN_W-INSPECTOR_MARGIN-CAT_GLYPH_WIDTH*7, cursor_y, "$%.5d\n", CAT_inventory_count(coin_item));
	cursor_y += 6;

	int box_h = CAT_LCD_SCREEN_H-INSPECTOR_MARGIN-84-cursor_y;
	CAT_set_sprite_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y);
	for(int scale = 1; scale <= 6; scale++)
	{
		if
		(
			item->sprite->width * scale <= 240 &&
			item->sprite->height * scale <= box_h
		)
			CAT_set_sprite_scale(scale);
	}
	CAT_set_sprite_mask(INSPECTOR_MARGIN, cursor_y, CAT_LCD_SCREEN_W-INSPECTOR_MARGIN, cursor_y + box_h);
	CAT_draw_sprite(item->sprite, 0, 120, cursor_y + box_h/2);
	CAT_strokeberry(INSPECTOR_MARGIN, cursor_y, CAT_LCD_SCREEN_W-INSPECTOR_MARGIN * 2, box_h, CAT_WHITE);
	cursor_y += box_h;
	if(!sale_lock || CAT_pulse(0.5f))
	{
		CAT_set_text_colour(sale_lock ? CHECKOUT_GOLD_COLOUR : CAT_WHITE);
		CAT_draw_textf(INSPECTOR_MARGIN, cursor_y + 2, "SELLING %d", sale_qty);
	}

	cursor_y += 32;

	if(sale_qty > 0)
	{
		uint16_t colour = sale_lock ? CHECKOUT_GOLD_COLOUR : CAT_WHITE;

		CAT_set_text_scale(2);
		CAT_set_text_colour(colour);
		CAT_set_text_flags(CAT_TEXT_FLAG_CENTER);
		cursor_y = CAT_draw_textf(120, cursor_y, "%d", sale_qty);

		int size = 24;
		float progress = CAT_ease_out_quart(sale_progress);
		int dist = 24 + progress * 40;
		CAT_draw_arrows(120, cursor_y + 12, size, dist, colour);
	}
	else
	{
		CAT_set_text_scale(2);
		CAT_set_text_colour(CAT_WHITE);
		CAT_set_text_flags(CAT_TEXT_FLAG_CENTER | CAT_TEXT_FLAG_WRAP);
		cursor_y = CAT_draw_text(120, cursor_y-8, "Insufficient stock!");
	}
}

static void buy_proc(int item_id)
{
	CAT_bind_checkout(item_id);
	CAT_pushdown_push(CAT_MS_checkout);
}

static void sell_proc(int item_id)
{
	CAT_bind_sale(item_id);
	CAT_pushdown_push(CAT_MS_sale);
}

void CAT_MS_shop(CAT_FSM_signal signal)
{
	switch(signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		{
			CAT_set_render_callback(CAT_render_shop);
			CAT_gui_begin_item_grid_context(true);
			break;
		}
		case CAT_FSM_SIGNAL_TICK:
		{			
			CAT_gui_begin_item_grid();
			for(int i = 0; i < NUM_TABS; i++)
				CAT_gui_item_grid_add_tab(tabs[i].title, NULL, i == NUM_TABS-1 ? sell_proc : buy_proc);

			for(int i = 0; i < item_table.length; i++)
			{
				if(!tabs[CAT_gui_item_grid_get_tab()].filter(i))
					continue;
				CAT_gui_item_grid_cell(i);
			}
			break;
		}

		case CAT_FSM_SIGNAL_EXIT:
		break;
	}
}

void CAT_render_shop()
{
	// GUI
}