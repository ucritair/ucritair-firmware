#include "cat_gui.h"

#include "cat_render.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "cat_machine.h"
#include <math.h>
#include <ctype.h>
#include "cat_input.h"
#include "cat_item.h"
#include "cat_item.h"
#include "sprite_assets.h"
#include "cat_gui.h"
#include "cat_structures.h"
#include "cat_gizmos.h"
#include "cat_core.h"
#include "cat_curves.h"
#include "cat_text.h"
#include "cat_alloc.h"
#include <stdarg.h>

//////////////////////////////////////////////////////////////////////////
// KEYBOARD

static const char** typecases[] = 
{
	(const char*[]) {
		"1234567890\x06",
		"qwertyuiop_",
		"asdfghjkl:'",
		"zxcvbnm ,.\x08"
	},
	(const char*[]) {
		"!?$%()+-*/\x06",
		"QWERTYUIOP_",
		"ASDFGHJKL;\"",
		"ZXCVBNM ,.\x08"
	},
};
#define TYPECASE_ROWS 4

static char* keyb_target = NULL;
static char keyb_buffer[128];
static int keyb_cursor;
static size_t keyb_max_size;
static enum {KEYB_KEYS, KEYB_BUTTONS} keyb_section;

static int keyb_case;
static int keyb_row;
static int keyb_glyph;

static void keyb_save_proc()
{
	strncpy(keyb_target, keyb_buffer, keyb_max_size);
	keyb_target = NULL;
}

static void keyb_quit_proc()
{
	keyb_target = NULL;
}

static struct
{
	const char* title;
	void (*proc)();
} keyb_buttons[] =
{
	{
		.title = "SAVE",
		.proc = keyb_save_proc
	},
	{
		.title = "CANCEL",
		.proc = keyb_quit_proc
	}
};
#define KEYB_BUTTON_COUNT (sizeof(keyb_buttons)/sizeof(keyb_buttons[0]))
static int keyb_button;

static bool keyb_show_cursor;
static int keyb_cursor_frame;

void CAT_gui_open_keyboard(char* target, size_t max_size)
{
	if(keyb_target != NULL)
		return;

	keyb_target = target;

	int length = strlen(target);
	strncpy(keyb_buffer, target, sizeof(keyb_buffer));
	keyb_cursor = length;
	keyb_max_size = max_size;
	keyb_section = KEYB_KEYS;

	keyb_case = 0;
	keyb_row = 0;
	keyb_glyph = 0;

	keyb_button = 0;

	keyb_show_cursor = true;
	keyb_cursor_frame = 0;

	CAT_input_clear();
}

bool CAT_gui_keyboard_is_open()
{
	return keyb_target != NULL;
}

void CAT_gui_keyboard_logic()
{
	if(keyb_section == KEYB_KEYS)
	{
		const char** typecase = typecases[keyb_case];

		if(CAT_input_pulse(CAT_BUTTON_UP))
			keyb_row -= 1;
		if(CAT_input_pulse(CAT_BUTTON_DOWN))
		{
			if(keyb_row >= TYPECASE_ROWS-1)
				keyb_section = KEYB_BUTTONS;
			else
				keyb_row += 1;
		}
		keyb_row = CAT_clamp(keyb_row, 0, TYPECASE_ROWS-1);
		const char* row = typecase[keyb_row];

		if(CAT_input_pulse(CAT_BUTTON_RIGHT))
			keyb_glyph += 1;
		if(CAT_input_pulse(CAT_BUTTON_LEFT))
			keyb_glyph -= 1;
		keyb_glyph = CAT_clamp(keyb_glyph, 0, strlen(row)-1);
		char glyph = row[keyb_glyph];

		if(CAT_input_pressed(CAT_BUTTON_A))
		{
			switch (glyph)
			{
				case 6:
				{
					keyb_cursor = CAT_max(keyb_cursor-1, 0);
					keyb_show_cursor = true;
					keyb_cursor_frame = 0;
				}
				break;

				case 8:
				{
					keyb_case = (keyb_case+1)%2;
				}
				break;

				default:
				{
					if(keyb_cursor < keyb_max_size-1)
					{
						keyb_buffer[keyb_cursor] = glyph;
						keyb_cursor += 1;
					}
				}
				break;
			}	
		}
		else if(CAT_input_pressed(CAT_BUTTON_B))
		{
			if(strlen(keyb_buffer) > 0)
			{
				keyb_cursor = CAT_max(keyb_cursor-1, 0);
				keyb_show_cursor = true;
				keyb_cursor_frame = 0;
			}
			else
			{
				keyb_button = 1;
				keyb_section = KEYB_BUTTONS;	
			}
		}
		else if(CAT_input_pressed(CAT_BUTTON_SELECT))
		{
			keyb_case = (keyb_case+1)%2;
		}

		keyb_buffer[keyb_cursor] = '\0';
	}
	else if(keyb_section == KEYB_BUTTONS)
	{
		if(CAT_input_pressed(CAT_BUTTON_UP))
			keyb_section = KEYB_KEYS;

		if(CAT_input_pressed(CAT_BUTTON_LEFT))
			keyb_button -= 1;
		if(CAT_input_pressed(CAT_BUTTON_RIGHT))
			keyb_button += 1;
		keyb_button = CAT_wrap(keyb_button, KEYB_BUTTON_COUNT);

		if(CAT_input_pressed(CAT_BUTTON_A) || CAT_input_pressed(CAT_BUTTON_B))
			keyb_buttons[keyb_button].proc();
	}
	
	keyb_cursor_frame += 1;
	if(keyb_cursor_frame >= 4)
	{
		keyb_show_cursor = !keyb_show_cursor;
		keyb_cursor_frame = 0;
	}
}

#define KEYB_Y0 (CAT_LCD_SCREEN_H/2)
#define KEYB_Y1 (CAT_LCD_SCREEN_H-1)
#define KEYB_PAD 4

void CAT_gui_keyboard()
{
	CAT_rowberry(KEYB_Y0, KEYB_Y1, CAT_WHITE);
	CAT_lineberry(0, KEYB_Y0, CAT_LCD_SCREEN_W, KEYB_Y0, CAT_192_GREY);
	int cursor_y = KEYB_Y0 + KEYB_PAD;

	int line = CAT_LINE_CAPACITY(KEYB_PAD, KEYB_PAD+CAT_GLYPH_WIDTH, CAT_GLYPH_WIDTH);
	int overshoot = CAT_max((int) strlen(keyb_buffer) - line, 0);
	CAT_draw_textf_depr(KEYB_PAD - overshoot * CAT_GLYPH_WIDTH, cursor_y, "%s%s", keyb_buffer, keyb_show_cursor ? "|" : "");
	cursor_y += CAT_GLYPH_HEIGHT + KEYB_PAD;

	CAT_lineberry(0, cursor_y, CAT_LCD_SCREEN_W, cursor_y, CAT_192_GREY);
	cursor_y += KEYB_PAD*2;

	int cursor_x = KEYB_PAD * 2;
	const char** typecase = typecases[keyb_case];
	for(int i = 0; i < TYPECASE_ROWS; i++)
	{
		const char* row = typecase[i];
		for(int j = 0; j < strlen(row); j++)
		{
			char glyph = row[j];
			CAT_draw_sprite(&glyph_sprite, glyph, cursor_x + 2, cursor_y + 3);

			uint16_t outline_c = i == keyb_row && j == keyb_glyph && keyb_section == KEYB_KEYS ?
			CAT_BLACK : CAT_192_GREY;
			CAT_strokeberry(cursor_x, cursor_y, CAT_GLYPH_WIDTH + 4, CAT_GLYPH_HEIGHT + 6, outline_c);

			cursor_x += CAT_GLYPH_WIDTH + 8;
		}
		cursor_y += CAT_GLYPH_HEIGHT + 8;
		cursor_x = KEYB_PAD * 2;
	}
	cursor_y += KEYB_PAD;

	for(int i = 0; i < KEYB_BUTTON_COUNT; i++)
	{
		int x = cursor_x;
		int y = cursor_y;
		int w = strlen(keyb_buttons[i].title) * CAT_GLYPH_WIDTH + KEYB_PAD;
		int h = CAT_GLYPH_HEIGHT + KEYB_PAD;

		CAT_draw_text_depr(cursor_x+KEYB_PAD/2, cursor_y+KEYB_PAD/2, keyb_buttons[i].title);

		uint16_t outline_c = keyb_section == KEYB_BUTTONS && keyb_button == i ?
		CAT_BLACK : CAT_192_GREY;
		CAT_strokeberry(x, y, w, h, outline_c);

		cursor_x += w + KEYB_PAD;
	}
}


//////////////////////////////////////////////////////////////////////////
// ITEM GRID

#define ITEM_GRID_MARGIN 12
#define ITEM_GRID_CELL_SIZE 64
#define ITEM_GRID_PAD 8

#define ITEM_GRID_COLS ((CAT_LCD_SCREEN_W-ITEM_GRID_MARGIN) / (ITEM_GRID_CELL_SIZE + ITEM_GRID_PAD))
#define ITEM_GRID_ROWS ((CAT_LCD_SCREEN_H-ITEM_GRID_MARGIN) / (ITEM_GRID_CELL_SIZE + ITEM_GRID_PAD))
#define ITEM_GRID_CELLS (ITEM_GRID_ROWS * ITEM_GRID_COLS)

#define ITEM_GRID_X ((CAT_LCD_SCREEN_W - (ITEM_GRID_COLS*ITEM_GRID_CELL_SIZE + (ITEM_GRID_COLS-1)*ITEM_GRID_PAD))/2)
#define ITEM_GRID_Y ((CAT_LCD_SCREEN_H-ITEM_GRID_PAD) - (ITEM_GRID_ROWS*ITEM_GRID_CELL_SIZE + (ITEM_GRID_ROWS-1)*ITEM_GRID_PAD))

#define ITEM_GRID_NAV_W CAT_LCD_SCREEN_W
#define ITEM_GRID_NAV_H CAT_LCD_SCREEN_H / 4
#define ITEM_GRID_NAV_X 0
#define ITEM_GRID_NAV_Y ((CAT_LCD_SCREEN_H - ITEM_GRID_NAV_H)/2)

#define ITEM_GRID_SELECT_TIME 1
#define ITEM_GRID_BG_COLOUR 0xbdb4
#define ITEM_GRID_NAV_COLOUR ITEM_GRID_BG_COLOUR
#define ITEM_GRID_ACTION_COLOUR CAT_RGB8882565(255 - 0, 255 - 141, 255 - 141)

static bool item_grid_status = false;
static bool item_grid_handle_exit = false;
static bool item_grid_reset = false;

static int item_grid_pool_backing[CAT_ITEM_TABLE_CAPACITY];
static CAT_int_list item_grid_pool;
static struct item_grid_datum
{
	bool highlight;
} item_grid_data[CAT_ITEM_TABLE_CAPACITY];
static int item_grid_selector = 0;

static int item_grid_confirmed = false;
static int item_grid_confirm_frame = 0;

enum
{
	GRID,
	NAV
};
static int item_grid_focus = GRID;

typedef struct
{
	const char* title;
	CAT_item_proc focus_proc;
	CAT_item_proc confirm_proc;
} item_grid_tab;
static item_grid_tab item_grid_tab_list[8];
int item_grid_tab_count = 0;
int item_grid_tab_idx = 0;

int item_grid_get_page()
{
	return item_grid_selector / ITEM_GRID_CELLS * ITEM_GRID_CELLS;
}

bool item_grid_is_empty()
{
	return item_grid_pool.length == 0;
}

void CAT_gui_begin_item_grid_context(bool handle_exit)
{
	item_grid_status = false;

	CAT_ilist(&item_grid_pool, item_grid_pool_backing, CAT_ITEM_TABLE_CAPACITY);
	item_grid_confirmed = false;
	item_grid_confirm_frame = 0;

	item_grid_focus = GRID;
	item_grid_tab_count = 0;

	item_grid_handle_exit = handle_exit;

	if(item_grid_reset)
	{
		item_grid_tab_idx = 0;
		item_grid_selector = 0;
	}
	item_grid_reset = false;
}

void CAT_gui_begin_item_grid()
{
	item_grid_status = true;

	CAT_ilist(&item_grid_pool, item_grid_pool_backing, CAT_ITEM_TABLE_CAPACITY);

	item_grid_tab_count = 0;
}

void CAT_gui_item_grid_add_tab(const char* title, CAT_item_proc focus_proc, CAT_item_proc confirm_proc)
{
	if(item_grid_tab_count >= 8)
		return;
	item_grid_tab* tab = &item_grid_tab_list[item_grid_tab_count];
	tab->title = title;
	tab->focus_proc = focus_proc;
	tab->confirm_proc = confirm_proc;
	item_grid_tab_count += 1;
}

int CAT_gui_item_grid_get_tab()
{
	return item_grid_tab_idx;
}

void CAT_gui_item_grid_cell(int item_id)
{
	if(item_id < 0 || item_id >= item_table.length)
		return;
	CAT_ilist_push(&item_grid_pool, item_id);
	int idx = item_grid_pool.length-1;
	item_grid_data[idx] = (struct item_grid_datum)
	{
		.highlight = false
	};
}

void CAT_gui_item_grid_highlight()
{
	item_grid_data[item_grid_pool.length-1].highlight = true;
}

void CAT_gui_item_grid_logic()
{
	switch (item_grid_focus)
	{
		case NAV:
			if(item_grid_tab_count > 0)
			{
				int tab_delta = 0;
				if(CAT_input_pressed(CAT_BUTTON_LEFT))
					tab_delta += -1;
				if(CAT_input_pressed(CAT_BUTTON_RIGHT))
					tab_delta += 1;
				item_grid_tab_idx = CAT_wrap(item_grid_tab_idx+tab_delta, item_grid_tab_count);
				if(tab_delta != 0)
					item_grid_selector = 0;

				if(CAT_input_pressed(CAT_BUTTON_A) || CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_DOWN))
					item_grid_focus = GRID;
			}
		break;
	
		case GRID:
		{
			if(!item_grid_confirmed)
			{
				int delta = 0;
				if(CAT_input_pulse(CAT_BUTTON_LEFT))
					delta += -1;
				if(CAT_input_pulse(CAT_BUTTON_RIGHT))
					delta += 1;
				if(CAT_input_pulse(CAT_BUTTON_UP))
				{
					if(item_grid_selector < 3 && item_grid_tab_count > 1)
						item_grid_focus = NAV;
					else
						delta += -ITEM_GRID_COLS;
				}
				if(CAT_input_pulse(CAT_BUTTON_DOWN))
					delta += ITEM_GRID_COLS;
				item_grid_selector = CAT_clamp(item_grid_selector+delta, 0, item_grid_pool.length-1);

				if(CAT_input_pressed(CAT_BUTTON_A))
					item_grid_confirmed = true;
			}
			else if(item_grid_confirm_frame < 3)
				item_grid_confirm_frame += 1;
			else if(!item_grid_is_empty())
			{
				item_grid_tab_list[item_grid_tab_idx].confirm_proc(item_grid_pool.data[item_grid_selector]);
				item_grid_confirmed = false;
				item_grid_confirm_frame = 0;
			}

			if(CAT_input_pressed(CAT_BUTTON_B) && item_grid_handle_exit)
			{
				item_grid_reset = true;
				CAT_pushdown_pop();
			}
		}
		break;
	}
}

bool CAT_gui_item_grid_is_open()
{
	return item_grid_status;
}

void CAT_gui_item_grid()
{
	item_grid_tab* tab = &item_grid_tab_list[item_grid_tab_idx];

	CAT_frameberry(ITEM_GRID_BG_COLOUR);

	CAT_set_text_colour_depr(CAT_WHITE);
	CAT_set_text_mask_depr(4, 4, CAT_LCD_SCREEN_W-4, 4+26);
	const char* item_name = !item_grid_is_empty() ? item_table.data[item_grid_pool.data[item_grid_selector]].name : "";
	if(item_grid_tab_count > 1)
		CAT_draw_textf_depr(4, 8, ">>> %s/%s", tab->title, item_name);
	else
		CAT_draw_textf_depr(4, 8, ">>> %s", item_name);
	CAT_lineberry(4, 26, CAT_LCD_SCREEN_W-4, 26, CAT_WHITE);

	int x = ITEM_GRID_X;
	int y = ITEM_GRID_Y;
	int idx = item_grid_get_page();
	int idx_limit = CAT_min(item_grid_get_page() + ITEM_GRID_CELLS, item_grid_pool.length);

	while (idx < idx_limit)
	{
		for(int row = 0; row < ITEM_GRID_ROWS && idx < idx_limit; row++)
		{
			for (int col = 0; col < ITEM_GRID_COLS && idx < idx_limit; col++)
			{
				const CAT_sprite* bg = item_grid_data[idx].highlight ?
				&ui_item_frame_bg_spec_sprite : &ui_item_frame_bg_sprite;
				CAT_draw_sprite(bg, 0, x, y);

				CAT_item* item = CAT_get_item(item_grid_pool.data[idx]);
				float aspect = item->sprite->height / (float) item->sprite->width;
				int major_axis = CAT_max(item->sprite->width, item->sprite->height);

				int draw_y = y + ITEM_GRID_CELL_SIZE/2;
				if(aspect <= 1.25f)
					CAT_set_sprite_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y);
				else
				{
					draw_y = y + ITEM_GRID_CELL_SIZE - 4;
					CAT_set_sprite_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_BOTTOM);
				}

				for(int scale = 1; scale <= 3; scale++)
				{
					if(major_axis * scale <= ITEM_GRID_CELL_SIZE)
						CAT_set_sprite_scale(scale);
				}
				
				CAT_set_sprite_mask(x+6, y+6, x+ITEM_GRID_CELL_SIZE-6, y+ITEM_GRID_CELL_SIZE-6);
				CAT_draw_sprite(item->sprite, 0, x + ITEM_GRID_CELL_SIZE/2, draw_y);
				
				if(idx == item_grid_selector)
				{
					if(!item_grid_confirmed ||item_grid_confirm_frame & 1 || tab->confirm_proc == NULL)
						CAT_draw_sprite(&ui_item_frame_fg_sprite, 0, x, y);
				}

				idx += 1;
				x += ITEM_GRID_CELL_SIZE + ITEM_GRID_PAD;
			}
			x = ITEM_GRID_X;
			y += ITEM_GRID_CELL_SIZE + ITEM_GRID_PAD;
		}
	}

	if(item_grid_is_empty())
	{
		CAT_set_text_colour_depr(CAT_WHITE);
		CAT_draw_text_depr(4, y, "There's nothing here...");
	}

	if (item_grid_get_page() < (item_grid_pool.length-ITEM_GRID_CELLS))
		CAT_draw_sprite(&ui_down_arrow_sprite, -1, 240 - 32, 320 - 24);

	if(item_grid_focus == NAV)
	{
		CAT_fillberry(ITEM_GRID_NAV_X, ITEM_GRID_NAV_Y, ITEM_GRID_NAV_W, ITEM_GRID_NAV_H, ITEM_GRID_NAV_COLOUR);
		CAT_lineberry(ITEM_GRID_NAV_X, ITEM_GRID_NAV_Y, ITEM_GRID_NAV_X+ITEM_GRID_NAV_W, ITEM_GRID_NAV_Y, CAT_WHITE);
		CAT_lineberry(ITEM_GRID_NAV_X, ITEM_GRID_NAV_Y+ITEM_GRID_NAV_H, ITEM_GRID_NAV_X+ITEM_GRID_NAV_W, ITEM_GRID_NAV_Y+ITEM_GRID_NAV_H, CAT_WHITE);

		CAT_set_text_colour_depr(CAT_WHITE);
		CAT_set_text_scale_depr(2);
		CAT_set_text_flags_depr(CAT_TEXT_FLAG_CENTER);
		CAT_draw_text_depr(CAT_LCD_SCREEN_W/2, ITEM_GRID_NAV_Y + ITEM_GRID_NAV_H/2 - 12, tab->title);
		CAT_draw_arrows(CAT_LCD_SCREEN_W/2, ITEM_GRID_NAV_Y + ITEM_GRID_NAV_H/2, 12, ITEM_GRID_NAV_W - 48, CAT_WHITE);
		CAT_draw_arrows(CAT_LCD_SCREEN_W/2, ITEM_GRID_NAV_Y + ITEM_GRID_NAV_H/2, 12, ITEM_GRID_NAV_W - 40, CAT_WHITE);
		CAT_draw_arrows(CAT_LCD_SCREEN_W/2, ITEM_GRID_NAV_Y + ITEM_GRID_NAV_H/2, 12, ITEM_GRID_NAV_W - 32, CAT_WHITE);
	}

	if(CAT_is_last_render_cycle())
	{
		item_grid_status = false;
	}
}


//////////////////////////////////////////////////////////////////////////
// NEW FRAMEWORK

#define CAT_CRC32_IMPL
#include "cat_crc32.h"

typedef uint32_t GUI_ID;
#define GUI_ID_NULL 0

typedef struct GUI_node GUI_node;
struct GUI_node
{
	// HEADER
	GUI_ID parent;
	GUI_ID id;
	enum
	{
		GUI_NODE_TYPE_WINDOW,
		GUI_NODE_TYPE_TEXT,
		GUI_NODE_TYPE_OPTION,
		GUI_NODE_TYPE_IMAGE,
		GUI_NODE_TYPE_TITLE,
		GUI_NODE_TYPE_TOGGLE,
		GUI_NODE_TYPE_TICKER
	} type;
	bool alive;

	// PAYLOAD
	union
	{
		struct // : WINDOW
		{
			int x0, y0, x1, y1;
			GUI_node* children[16];
			int child_count;
			int selector;
			enum {WINDOW_SHOULD_CLOSE, WINDOW_SHOULD_OPEN, WINDOW_OPEN} status;
		};

		struct // : TEXT
		{
			char text[128];
		};

		struct // : OPTION
		{
			char text[32];
			bool trigger;
		} option;

		struct // : IMAGE
		{
			const CAT_sprite* sprite;
			int frame_idx;
		};

		struct // : TOGGLE
		{
			char text[32];
			bool trigger;
			bool cached_value;
			int style;
		} toggle;

		struct // : TICKER
		{
			char text[32];
			int delta;
			int cached_value;
		} ticker;
	};
};

void GUI_node_add_child(GUI_node* node, GUI_node* child)
{
	if(node->child_count >= 32)
		return;
	int idx = node->child_count;
	node->children[idx] = child;
	node->child_count += 1;
}

#define GUI_POOL_CAPACITY 256
static GUI_node GUI_pool[GUI_POOL_CAPACITY] = {0};

static void GUI_free_node(GUI_node* node)
{
	node->parent = GUI_ID_NULL;
	node->id = GUI_ID_NULL;
}

static void GUI_pool_free()
{
	for(int i = 0; i < GUI_POOL_CAPACITY; i++)
	{			
		switch (GUI_pool[i].type)
		{
			case GUI_NODE_TYPE_WINDOW:
			{
				if(GUI_pool[i].status == WINDOW_SHOULD_CLOSE)
					GUI_free_node(&GUI_pool[i]);
				GUI_pool[i].alive = false;
			}
			break;
		
			default:
			{
				if(!GUI_pool[i].alive)
					GUI_free_node(&GUI_pool[i]);
				GUI_pool[i].alive = false;
			}
			break;
		}
	}
}

static GUI_node* GUI_pool_get(GUI_ID id)
{
	for(int i = 0; i < GUI_POOL_CAPACITY; i++)
	{
		if(GUI_pool[i].id != GUI_ID_NULL)
		if(GUI_pool[i].id == id)
			return &GUI_pool[i];
	}
	return NULL;
}

static GUI_node* GUI_pool_get_free()
{
	for(int i = 0; i < GUI_POOL_CAPACITY; i++)
	{
		if(GUI_pool[i].parent == GUI_ID_NULL && GUI_pool[i].id == GUI_ID_NULL)
			return &GUI_pool[i];
	}
	return NULL;
}

static GUI_node* GUI_pool_register(GUI_ID id, bool persistent)
{
	GUI_node* destination = NULL;
	if(persistent)
		destination = GUI_pool_get(id);
	if(destination == NULL)
		destination = GUI_pool_get_free();
	return destination;
}

#define CAT_GUI_ID_STACK_CAPACITY 32
typedef struct GUI_ID_stack GUI_ID_stack;
struct GUI_ID_stack
{
	GUI_ID data[CAT_GUI_ID_STACK_CAPACITY];
	int count;
};

static void GUI_ID_stack_clear(GUI_ID_stack* stack)
{
	stack->count = 0;
}

static void GUI_ID_stack_push(GUI_ID_stack* stack, GUI_ID id)
{
	if(stack->count >= CAT_GUI_ID_STACK_CAPACITY)
		return;
	int idx = stack->count;
	if(stack->data[idx-1] != id)
	{
		stack->data[idx] = id;
		stack->count += 1;
	}
}

static GUI_ID GUI_ID_stack_pop(GUI_ID_stack* stack)
{
	if(stack->count <= 0)
		return GUI_ID_NULL;
	stack->count -= 1;
	return stack->data[stack->count];
}

static GUI_ID GUI_ID_stack_get(GUI_ID_stack* stack, int idx)
{
	if(idx < 0)
		idx += stack->count;
	if(idx >= stack->count)
		return GUI_ID_NULL;
	return stack->data[idx];
}

static GUI_ID_stack transient_stack = {0};
static GUI_ID_stack persistent_stack = {0};

void CAT_GUI_open_window(const char* text)
{
	GUI_ID parent = GUI_ID_NULL; // GUI_ID_stack_get(&transient_stack, -1);
	GUI_ID id = CAT_CRC32_hash(text, parent);

	GUI_node* node = GUI_pool_register(id, true);
	if(node == NULL)
		return;

	memset(node, 0, sizeof(GUI_node));
	node->parent = parent;
	node->id = id;
	node->type = GUI_NODE_TYPE_WINDOW;
	node->status = WINDOW_SHOULD_OPEN;
}

void CAT_GUI_close_current_window()
{
	GUI_ID head_id = GUI_ID_stack_pop(&persistent_stack);
	GUI_node* head = GUI_pool_get(head_id);

	if(head == NULL)
		return;
	head->status = WINDOW_SHOULD_CLOSE;

	CAT_input_clear();
}

bool CAT_GUI_begin_window(const char* text, int x0, int y0, int x1, int y1)
{	
	GUI_ID parent = GUI_ID_NULL; // GUI_ID_stack_get(&transient_stack, -1);
	GUI_ID id = CAT_CRC32_hash(text, parent);

	GUI_node* node = GUI_pool_get(id);
	if(node == NULL)
		return false;
	if(node->status == WINDOW_SHOULD_CLOSE)
		return false;

	node->x0 = x0;
	node->y0 = y0;
	node->x1 = x1;
	node->y1 = y1;
	node->child_count = 0;
	node->status = WINDOW_OPEN;

	GUI_ID_stack_push(&transient_stack, id);
	GUI_ID_stack_push(&persistent_stack, id);
	return true;
}

void CAT_GUI_end_window()
{
	GUI_ID_stack_pop(&transient_stack);
}

#define GUI_FMTCPY(buf, fmt) { \
	va_list args; \
	va_start(args, fmt); \
	vsnprintf(buf, sizeof(buf), fmt, args); \
	va_end(args); \
} \

void CAT_GUI_text(const char* fmt, ...)
{
	GUI_ID parent_id = GUI_ID_stack_get(&transient_stack, -1);
	GUI_node* parent_node = GUI_pool_get(parent_id);
	if(parent_node == NULL)
		return;

	GUI_node* node = GUI_pool_register(GUI_ID_NULL, false);
	if(node == NULL)
		return;

	memset(node, 0, sizeof(GUI_node));
	node->parent = parent_id;
	node->type = GUI_NODE_TYPE_TEXT;
	GUI_FMTCPY(node->text, fmt);

	GUI_node_add_child(parent_node, node);
}

bool CAT_GUI_option(const char* text)
{
	GUI_ID parent_id = GUI_ID_stack_get(&transient_stack, -1);
	GUI_node* parent_node = GUI_pool_get(parent_id);
	if(parent_node == NULL)
		return false;

	GUI_ID id = CAT_CRC32_hash(text, parent_id);
	GUI_node* node = GUI_pool_get(id);
	
	bool result = false;
	if(node != NULL)
		result = node->option.trigger;
	else
	{
		node = GUI_pool_register(id, true);	
		if(node == NULL)
			return false;
		memset(node, 0, sizeof(GUI_node));
	}

	node->parent = parent_id;
	node->id = id;
	node->type = GUI_NODE_TYPE_OPTION;
	strcpy(node->option.text, text);
	node->option.trigger = false;
	
	GUI_node_add_child(parent_node, node);
	return result;
}

void CAT_GUI_image(const CAT_sprite* sprite, int frame_idx)
{
	GUI_ID parent_id = GUI_ID_stack_get(&transient_stack, -1);
	GUI_node* parent_node = GUI_pool_get(parent_id);
	if(parent_node == NULL)
		return;

	GUI_node* node = GUI_pool_register(GUI_ID_NULL, false);
	if(node == NULL)
		return;

	memset(node, 0, sizeof(GUI_node));
	node->parent = parent_id;
	node->type = GUI_NODE_TYPE_IMAGE;
	node->sprite = sprite;
	node->frame_idx = frame_idx;

	GUI_node_add_child(parent_node, node);
}

void CAT_GUI_title(const char* fmt, ...)
{
	GUI_ID parent_id = GUI_ID_stack_get(&transient_stack, -1);
	GUI_node* parent_node = GUI_pool_get(parent_id);
	if(parent_node == NULL)
		return;

	GUI_node* node = GUI_pool_register(GUI_ID_NULL, false);
	if(node == NULL)
		return;

	memset(node, 0, sizeof(GUI_node));
	node->parent = parent_id;
	node->type = GUI_NODE_TYPE_TITLE;
	GUI_FMTCPY(node->text, fmt);

	GUI_node_add_child(parent_node, node);
}

bool CAT_GUI_toggle(const char* text, bool value, int style)
{
	GUI_ID parent_id = GUI_ID_stack_get(&transient_stack, -1);
	GUI_node* parent_node = GUI_pool_get(parent_id);
	if(parent_node == NULL)
		return false;

	GUI_ID id = CAT_CRC32_hash(text, parent_id);
	GUI_node* node = GUI_pool_get(id);
	
	bool result = value;
	if(node != NULL && node->toggle.trigger)
	{
		result = !result;
	}
	else
	{
		node = GUI_pool_register(id, true);	
		if(node == NULL)
			return result;
		memset(node, 0, sizeof(GUI_node));
	}

	node->parent = parent_id;
	node->id = id;
	node->type = GUI_NODE_TYPE_TOGGLE;
	strcpy(node->toggle.text, text);
	if(!node->toggle.trigger)
		node->toggle.cached_value = result;
	node->toggle.trigger = false;
	node->toggle.style = style;
	
	GUI_node_add_child(parent_node, node);
	return result;
}

int CAT_GUI_ticker(const char* text, int value, int min, int max)
{
	GUI_ID parent_id = GUI_ID_stack_get(&transient_stack, -1);
	GUI_node* parent_node = GUI_pool_get(parent_id);
	if(parent_node == NULL)
		return false;

	GUI_ID id = CAT_CRC32_hash(text, parent_id);
	GUI_node* node = GUI_pool_get(id);

	int result = value;
	if(node == NULL)
	{
		node = GUI_pool_register(id, true);	
		if(node == NULL)
			return result;
		memset(node, 0, sizeof(GUI_node));
	}
	result += node->ticker.delta;
	result = CAT_clamp(result, min, max);

	node->parent = parent_id;
	node->id = id;
	node->type = GUI_NODE_TYPE_TICKER;
	strcpy(node->ticker.text, text);
	node->ticker.delta = 0;
	node->ticker.cached_value = result;
	
	GUI_node_add_child(parent_node, node);
	return result;
}

void CAT_GUI_new_frame()
{
	GUI_pool_free();
	GUI_ID_stack_clear(&transient_stack);
	GUI_ID_stack_clear(&persistent_stack);
}

static int GUI_is_selectable(GUI_node* node)
{
	if(node->type == GUI_NODE_TYPE_OPTION)
		return true;
	if(node->type == GUI_NODE_TYPE_TOGGLE)
		return true;
	if(node->type == GUI_NODE_TYPE_TICKER)
		return true;
	return false;
}

static int GUI_count_selectables(GUI_node* node)
{
	int selectable_count = 0;
	for(int i = 0; i < node->child_count; i++)
	{
		if(GUI_is_selectable(node->children[i]))
			selectable_count += 1;
	}
	return selectable_count;
}

static GUI_node* GUI_get_selectable(GUI_node* node, int idx)
{
	int selectable_count = 0;
	for(int i = 0; i < node->child_count; i++)
	{
		if(GUI_is_selectable(node->children[i]))
		{
			if(selectable_count == idx)
				return node->children[i];
			selectable_count += 1;
		}
	}
	return NULL;
}

bool CAT_GUI_is_window_open(const char* text)
{
	// Is any window open
	if(text == NULL)
		return persistent_stack.count > 0;

	GUI_ID id = CAT_CRC32_hash(text, GUI_ID_NULL);
	GUI_node* node = GUI_pool_get(id);
	if(node == NULL)
		return false;
	return node->status != WINDOW_SHOULD_CLOSE;
}

int CAT_GUI_window_count()
{
	return persistent_stack.count;
}

void CAT_GUI_IO()
{
	for(int i = 0; i < persistent_stack.count; i++)
	{
		GUI_node* node = GUI_pool_get(persistent_stack.data[i]);
		node->alive = true;
		for(int j = 0; j < node->child_count; j++)
			node->children[j]->alive = true;
	}

	GUI_ID head_id = GUI_ID_stack_get(&persistent_stack, -1);
	GUI_node* head = GUI_pool_get(head_id);
	if(head == NULL)
		return;
	
	int selectable_count = GUI_count_selectables(head);
	if(selectable_count > 0)
	{
		if(CAT_input_pressed(CAT_BUTTON_UP))
			head->selector -= 1;
		if(CAT_input_pressed(CAT_BUTTON_DOWN))
			head->selector += 1;
		head->selector = CAT_wrap(head->selector, selectable_count);

		GUI_node* selected = GUI_get_selectable(head, head->selector);
		if(selected != NULL)
		{
			switch(selected->type)
			{
				case GUI_NODE_TYPE_OPTION:
				{
					if(CAT_input_pressed(CAT_BUTTON_A))
						selected->option.trigger = true;
					selectable_count += 1;
				}
				break;

				case GUI_NODE_TYPE_TOGGLE:
				{
					if(CAT_input_pressed(CAT_BUTTON_A))
						selected->toggle.trigger = true;
					selectable_count += 1;
				}
				break;

				case GUI_NODE_TYPE_TICKER:
				{
					if(CAT_input_pressed(CAT_BUTTON_LEFT))
						selected->ticker.delta -= 1;
					if(CAT_input_pressed(CAT_BUTTON_RIGHT))
						selected->ticker.delta += 1;
					selectable_count += 1;
				}
				break;

				default:
				{
					
				}
				break;
			}
		}
	}
	else
		head->selector = -1;
}

void GUI_draw_window(GUI_node* node)
{
	CAT_fillberry(node->x0, node->y0, node->x1-node->x0, node->y1-node->y0, CAT_WHITE);
	CAT_lineberry(node->x0-1, node->y0-1, node->x0-1, node->y1, CAT_GREY);
	CAT_lineberry(node->x0-1, node->y0-1, node->x1, node->y0-1, CAT_GREY);
	CAT_lineberry(node->x1, node->y0, node->x1, node->y1, CAT_BLACK);
	CAT_lineberry(node->x0, node->y1, node->x1, node->y1, CAT_BLACK);
	CAT_set_text_box
	(
		node->x0+4, node->y0+4,
		node->x1-4, node->y1-4
	);
}

void GUI_draw_text(GUI_node* node)
{
	CAT_text_box_draw(1, CAT_BLACK, node->text);
	CAT_text_box_newline(1);
}

void GUI_draw_option(GUI_node* node, bool selected)
{
	if(selected)
		CAT_text_box_draw(1, CAT_BLACK, "\1 %s <", node->option.text);
	else
		CAT_text_box_draw(1, CAT_BLACK, "\1 %s", node->option.text);
	CAT_text_box_newline(1);
}

void GUI_draw_image(GUI_node* node)
{
	int x = CAT_get_text_box_cursor_x();
	int y = CAT_get_text_box_cursor_y();
	CAT_draw_sprite(node->sprite, node->frame_idx, x, y);
	CAT_text_box_shift_cursor(0, node->sprite->height);
	CAT_text_box_reset_x();
	CAT_text_box_newline(1);
}

void GUI_draw_title(GUI_node* node)
{
	int x = CAT_get_text_box_cursor_x();
	int y = CAT_get_text_box_cursor_y();
	CAT_text_box_draw(1, CAT_BLACK, node->text);
	CAT_text_box_newline(1);
	CAT_text_box_shift_cursor(0, 2);

	y = CAT_get_text_box_cursor_y();
	CAT_rowberry(y, y+1, CAT_BLACK);
	CAT_text_box_shift_cursor(0, 6);	
}

void GUI_draw_toggle(GUI_node* node, bool selected)
{
	CAT_text_box_draw(1, CAT_BLACK, "\1 %s ", node->toggle.text);
	const CAT_sprite* sprite = node->toggle.style == CAT_GUI_TOGGLE_STYLE_CHECKBOX ? &ui_checkbox_sprite : &ui_radio_button_circle_sprite;
	int x = CAT_get_text_box_cursor_x();
	int y = CAT_get_text_box_cursor_y();
	int shift = (sprite->height - CAT_GLYPH_HEIGHT)/2;
	CAT_set_sprite_colour(CAT_BLACK);
	CAT_draw_sprite(sprite, node->toggle.cached_value, x, y-shift);
	CAT_text_box_shift_cursor(sprite->width + 4, 0);
	if(selected)
		CAT_text_box_draw(1, CAT_BLACK, "<");
	CAT_text_box_shift_cursor(0, shift+2);
	CAT_text_box_newline(1);
}

void GUI_draw_ticker(GUI_node* node, bool selected)
{
	CAT_text_box_draw(1, CAT_BLACK, "\1 %s <%d> %s", node->ticker.text, node->ticker.cached_value, selected ? "<" : "");
	CAT_text_box_newline(1);
}

void CAT_GUI_draw()
{
	for(int i = 0; i < persistent_stack.count; i++)
	{
		GUI_ID id = persistent_stack.data[i];
		GUI_node* node = GUI_pool_get(id);
		GUI_draw_window(node);

		for(int j = 0; j < node->child_count; j++)
		{
			GUI_node* child = node->children[j];
			if(child->type == GUI_NODE_TYPE_TITLE)
				GUI_draw_title(child);
		}

		int selectable_idx = 0;
		for(int j = 0; j < node->child_count; j++)
		{
			GUI_node* child = node->children[j];
			switch (child->type)
			{
				case GUI_NODE_TYPE_TEXT:
				{
					GUI_draw_text(child);
				}
				break;

				case GUI_NODE_TYPE_OPTION:
				{
					GUI_draw_option(child, selectable_idx == node->selector);
					selectable_idx += 1;
				}
				break;

				case GUI_NODE_TYPE_IMAGE:
				{
					GUI_draw_image(child);
				}
				break;

				case GUI_NODE_TYPE_TOGGLE:
				{
					GUI_draw_toggle(child, selectable_idx == node->selector);
					selectable_idx += 1;
				}
				break;

				case GUI_NODE_TYPE_TICKER:
				{
					GUI_draw_ticker(child, selectable_idx == node->selector);
					selectable_idx += 1;
				}
				break;

				default:
				{
				}
				break;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// MENU

static bool menu_exit_flag = false;

bool CAT_gui_begin_menu(const char* title)
{
	bool clicked = CAT_GUI_option(title);
	if(!CAT_GUI_is_window_open(title))
	{
		if(!CAT_GUI_is_window_open(NULL) || clicked)
			CAT_GUI_open_window(title);
	}

	bool open = CAT_GUI_begin_window(title, 0, 0, CAT_LCD_SCREEN_W, CAT_LCD_SCREEN_H);
	if(open)
	{
		CAT_GUI_title(title);
	}

	return open;
}

void CAT_gui_end_menu()
{
	CAT_GUI_end_window();
}

bool CAT_gui_menu_item(const char* title)
{
	return CAT_GUI_option(title);
}

bool CAT_gui_menu_toggle(const char* title, bool toggle, CAT_gui_toggle_style style)
{
	return CAT_GUI_toggle(title, toggle, style);
}

int CAT_gui_menu_ticker(const char* title, int value, int min, int max)
{
	return CAT_GUI_ticker(title, value, min, max);
}

void CAT_gui_menu_text(const char* title)
{
	CAT_GUI_text(title);
}

bool CAT_gui_menu_is_open()
{
	CAT_GUI_is_window_open(NULL);
}

void CAT_gui_menu_logic()
{
	if(CAT_input_pressed(CAT_BUTTON_B))
	{
		if(persistent_stack.count > 1)
			CAT_GUI_close_current_window();
	}
}


//////////////////////////////////////////////////////////////////////////
// FINALIZATION

void CAT_gui_tick()
{
	if(CAT_gui_keyboard_is_open())
		CAT_gui_keyboard_logic();
	else
	{
		if(CAT_gui_item_grid_is_open())
			CAT_gui_item_grid_logic();
		CAT_gui_menu_logic();
		CAT_GUI_IO();
	}	
}

void CAT_gui_render()
{
	if(CAT_gui_item_grid_is_open())
		CAT_gui_item_grid();
	CAT_GUI_draw();
	if(CAT_gui_keyboard_is_open())
		CAT_gui_keyboard();
}