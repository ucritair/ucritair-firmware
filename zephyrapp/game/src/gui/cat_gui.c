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
// POPUP

static const char* popup_text;
static int popup_style;
static int popup_selector;
static bool popup_result;
static bool popup_open = false;

void CAT_gui_open_popup(const char* text, int style)
{	
	popup_text = text;
	popup_style = style;
	popup_selector = 1;
	popup_result = false;
	popup_open = true;

	CAT_input_clear();
}

bool CAT_gui_popup_is_open()
{
	return popup_open;
}

void CAT_gui_popup_logic()
{
	if(popup_style == CAT_POPUP_STYLE_YES_NO)
	{
		if(CAT_input_pressed(CAT_BUTTON_LEFT))
			popup_selector = 0;
		if(CAT_input_pressed(CAT_BUTTON_RIGHT))
			popup_selector = 1;
		popup_result = popup_selector == 0;
		
		if(CAT_input_pressed(CAT_BUTTON_START) || CAT_input_pressed(CAT_BUTTON_B))
		{
			popup_result = false;
			popup_open = false;
		}
	}
	else
	{
		popup_selector = 0;
		popup_result = false;
	}

	if(CAT_input_pressed(CAT_BUTTON_A))
		popup_open = false;
}

#define POPUP_TILE_X 2
#define POPUP_TILE_Y 6
#define POPUP_TILE_W 11
#define POPUP_TILE_H 8
#define POPUP_X (POPUP_TILE_X * CAT_TILE_SIZE)
#define POPUP_Y (POPUP_TILE_Y * CAT_TILE_SIZE)
#define POPUP_W (POPUP_TILE_W * CAT_TILE_SIZE)
#define POPUP_H (POPUP_TILE_H * CAT_TILE_SIZE)
#define POPUP_MARGIN 8

void CAT_gui_popup()
{
	CAT_fillberry(POPUP_X, POPUP_Y, POPUP_W, POPUP_H, CAT_WHITE);
	CAT_strokeberry(POPUP_X, POPUP_Y, POPUP_W, POPUP_H, CAT_BLACK);
	CAT_strokeberry(POPUP_X-1, POPUP_Y-1, POPUP_W+2, POPUP_H+2, CAT_GREY);
	
	CAT_set_text_mask_depr(POPUP_X+POPUP_MARGIN, -1, POPUP_X+POPUP_W-POPUP_MARGIN, -1);
	CAT_set_text_flags_depr(CAT_TEXT_FLAG_WRAP);
	CAT_draw_text_depr(POPUP_X+POPUP_MARGIN, POPUP_Y+POPUP_MARGIN, popup_text);

	const char* responses =
	popup_style == CAT_POPUP_STYLE_YES_NO ?
	(popup_selector == 0 ? "[YES]   NO \n" : " YES   [NO]\n") :
	"[OKAY]";

	CAT_draw_text_depr
	(
		POPUP_X+POPUP_MARGIN, POPUP_Y+POPUP_H-POPUP_MARGIN-CAT_GLYPH_HEIGHT,
		responses
	);
}

bool CAT_gui_consume_popup()
{
	if(popup_open)
		return false;
	bool result = popup_result;
	popup_result = false;
	return result;
}


//////////////////////////////////////////////////////////////////////////
// MENU

typedef struct menu_node
{
	bool live;

	char title[32];
	bool clicked;
	
	CAT_gui_menu_type type;
	union 
	{
		struct
		{
			bool toggle;
			int style;
		} toggle_data;
		
		struct
		{
			int value;
			int min;
			int max;
			bool changed;
		} ticker_data;
	};
	
	int16_t parent;
	uint8_t children[16];
	uint8_t child_count;
	int8_t selector;
} menu_node;

static menu_node menu_table[256];
#define MENU_TABLE_SIZE (sizeof(menu_table) / sizeof(menu_table[0]))
static uint16_t menu_stack[24];
#define MENU_STACK_SIZE (sizeof(menu_stack) / sizeof(menu_stack[0]))
static uint8_t menu_stack_length = 0;
static int menu_root = -1;
static bool menu_reset = false;
static void (*menu_exit_proc)() = NULL;
static bool menu_wrap = true;

void CAT_gui_menu_override_exit(void (*exit_proc)())
{
	menu_exit_proc = exit_proc;
}

void CAT_gui_menu_force_reset()
{
	menu_reset = true;
}

void CAT_gui_menu_disable_wrap()
{
	menu_wrap = false;
}

uint16_t register_menu_node(const char* title, CAT_gui_menu_type type)
{
	uint16_t idx = CAT_hash_string(title) % MENU_TABLE_SIZE;
	while (menu_table[idx].live)
		idx++;

	if(idx < 0 || idx >= MENU_TABLE_SIZE)
	{
		CAT_printf("[ERROR] Menu node index %d is invalid", idx);
		return UINT16_MAX;
	}

	menu_table[idx] = (menu_node)
	{
		.live = true,
		.clicked = false,

		.type = type,

		.parent = -1,
		.child_count = 0,
		.selector = 0
	};
	strncpy(menu_table[idx].title, title, sizeof(menu_table[idx].title));

	return idx;
}

int find_menu_node(const char* title)
{
	uint16_t idx = CAT_hash_string(title) % MENU_TABLE_SIZE;
	while (menu_table[idx].live)
	{
		if (strncmp(menu_table[idx].title, title, sizeof(menu_table[idx].title)) == 0)
			return idx;
		idx++;
	}
	return -1;
}

void push_menu_node(uint16_t table_idx)
{
	if(menu_stack_length >= MENU_STACK_SIZE)
	{
		CAT_printf("[ERROR] Attempted to add to full menu stack!\n");
		return;
	}

	menu_stack[menu_stack_length] = table_idx;
	menu_stack_length += 1;
}

uint16_t pop_menu_node()
{
	menu_stack_length -= 1;
	return menu_stack[menu_stack_length];
}

menu_node* get_local_head()
{
	return &menu_table[menu_stack[menu_stack_length-1]];
}

menu_node* get_global_head()
{
	if(menu_root == -1)
		return NULL;
	menu_node* ptr = &menu_table[menu_root];
	for(int i = 0; i < ptr->child_count; i++)
	{
		menu_node* child = &menu_table[ptr->children[i]];
		if(child->clicked && child->child_count > 0)
		{
			ptr = child;
			i = -1;
			continue;
		}
	}
	return ptr;
}

uint8_t menu_add_child(uint16_t table_idx)
{
	menu_node* head = get_local_head();
	uint8_t child_idx = head->child_count;
	if(child_idx >= 32)
	{
		CAT_printf("[ERROR] Attempted to add child to full menu node!\n");
		return 255;
	}

	head->children[child_idx] = table_idx;
	head->child_count += 1;
	menu_table[table_idx].parent = menu_stack[menu_stack_length-1];
	return child_idx;
}

bool consume_click(uint16_t table_idx)
{
	menu_node* node = &menu_table[table_idx];
	bool value = node->clicked;
	node->clicked = false;
	return value;
}

bool CAT_gui_begin_menu(const char* title)
{
	if(menu_reset)
	{
		for(int i = 0; i < MENU_TABLE_SIZE; i++)
		{
			menu_table[i].live = false;
			menu_table[i].clicked = false;
			menu_table[i].parent = -1;
			menu_table[i].child_count = 0;
			menu_table[i].selector = 0;
		}
		menu_reset = false;
		menu_exit_proc = NULL;
		menu_wrap = true;
	}

	int idx = find_menu_node(title);
	if(idx == -1)
		idx = register_menu_node(title, CAT_GUI_MENU_TYPE_DEFAULT);
	menu_table[idx].child_count = 0;

	if (!CAT_gui_menu_is_open())
	{
		menu_root = idx;
		menu_table[idx].clicked = true;
		menu_table[idx].parent = -1;
		push_menu_node(idx);
	}
	else
	{
		menu_add_child(idx);
		if(menu_table[idx].clicked)
			push_menu_node(idx);
	}
	
	return menu_table[idx].clicked;
}

void CAT_gui_end_menu()
{
	pop_menu_node();	
}

bool CAT_gui_menu_is_open()
{
	return menu_root != -1;
}

bool CAT_gui_menu_item(const char* title)
{
	int idx = find_menu_node(title);
	if(idx == -1)
		idx = register_menu_node(title, CAT_GUI_MENU_TYPE_DEFAULT);

	if(menu_table[idx].type != CAT_GUI_MENU_TYPE_DEFAULT)
		menu_table[idx].type = CAT_GUI_MENU_TYPE_DEFAULT;

	menu_add_child(idx);

	return consume_click(idx);
}

bool CAT_gui_menu_toggle(const char* title, bool toggle, CAT_gui_toggle_style style)
{	
	int idx = find_menu_node(title);
	if(idx == -1)
		idx = register_menu_node(title, CAT_GUI_MENU_TYPE_TOGGLE);
	menu_add_child(idx);
	menu_node* node = &menu_table[idx];

	if(node->type != CAT_GUI_MENU_TYPE_TOGGLE)
		node->type = CAT_GUI_MENU_TYPE_TOGGLE;

	node->toggle_data.toggle = toggle;
	node->toggle_data.style = style;

	return consume_click(idx);
}

int CAT_gui_menu_ticker(const char* title, int value, int min, int max)
{	
	int idx = find_menu_node(title);
	bool first_call = idx == -1;
	if(first_call)
		idx = register_menu_node(title, CAT_GUI_MENU_TYPE_TICKER);
	menu_add_child(idx);
	menu_node* node = &menu_table[idx];

	if(node->type != CAT_GUI_MENU_TYPE_TICKER)
		node->type = CAT_GUI_MENU_TYPE_TICKER;

	if(first_call || (node->ticker_data.value != value && !node->ticker_data.changed))
		node->ticker_data.value = value;
	node->ticker_data.min = min;
	node->ticker_data.max = max;

	if(node->ticker_data.changed)
	{
		node->ticker_data.changed = false;
		return node->ticker_data.value;
	}
	return value;
}

void CAT_gui_menu_text(const char* title)
{
	int idx = find_menu_node(title);
	if(idx == -1)
		idx = register_menu_node(title, CAT_GUI_MENU_TYPE_TEXT);

	if(menu_table[idx].type != CAT_GUI_MENU_TYPE_TEXT)
		menu_table[idx].type = CAT_GUI_MENU_TYPE_TEXT;

	menu_add_child(idx);
}

void CAT_gui_menu_logic()
{
	menu_node* head = get_global_head();

	if(CAT_input_pressed(CAT_BUTTON_UP))
		head->selector -= 1;
	if(CAT_input_pressed(CAT_BUTTON_DOWN))
		head->selector += 1;
	if(menu_wrap)
		head->selector = CAT_wrap(head->selector, head->child_count);
	else
		head->selector = CAT_clamp(head->selector, 0, head->child_count-1);

	while
	(
		menu_table[head->children[head->selector]].type == CAT_GUI_MENU_TYPE_TEXT &&
		head->selector < head->child_count
	)
	{
		head->selector += 1;
	}

	menu_node* selected = &menu_table[head->children[head->selector]];

	switch (selected->type)
	{
		case CAT_GUI_MENU_TYPE_TICKER:
		{
			if(CAT_input_pulse(CAT_BUTTON_LEFT))
			{
				selected->ticker_data.value -= 1;
				selected->ticker_data.changed = true;
			}
			if(CAT_input_pulse(CAT_BUTTON_RIGHT))
			{
				selected->ticker_data.value += 1;
				selected->ticker_data.changed = true;
			}
			selected->ticker_data.value = CAT_clamp
			(
				selected->ticker_data.value,
				selected->ticker_data.min,
				selected->ticker_data.max
			);
		}
		break;

		default:
		break;
	}

	if(CAT_input_pressed(CAT_BUTTON_A))
		selected->clicked = true;
	if(CAT_input_pressed(CAT_BUTTON_B))
	{
		if(head->parent == -1)
		{
			menu_reset = true;
			if(menu_exit_proc == NULL)
				CAT_pushdown_pop();
			else
				menu_exit_proc();
		}
		else
		{
			head->clicked = false;
		}
	}
}

#define MENU_PAD 8

void CAT_gui_menu()
{
	menu_node* head = get_global_head();

	CAT_frameberry(CAT_WHITE);
	
	CAT_set_text_box(MENU_PAD, MENU_PAD, CAT_LCD_SCREEN_W-MENU_PAD, CAT_LCD_SCREEN_H-MENU_PAD);
	CAT_text_box_draw(1, CAT_BLACK, "%s\n", head->title);
	CAT_text_box_shift_cursor(0, MENU_PAD);

	CAT_rowberry(CAT_get_text_box_cursor_y(), CAT_get_text_box_cursor_y()+1, CAT_BLACK);
	CAT_text_box_shift_cursor(0, MENU_PAD);

	for(int i = 0; i < head->child_count; i++)
	{
		menu_node* child = &menu_table[head->children[i]];

		if(child->type == CAT_GUI_MENU_TYPE_TEXT)
			CAT_text_box_draw(1, CAT_BLACK, "%s ", child->title);
		else
			CAT_text_box_draw(1, CAT_BLACK, "\1 %s ", child->title);;

		switch (child->type)
		{
			case CAT_GUI_MENU_TYPE_TOGGLE:
			{
				const CAT_sprite* sprite =
				child->toggle_data.style == CAT_GUI_TOGGLE_STYLE_CHECKBOX ?
				&ui_checkbox_sprite : &ui_radio_button_circle_sprite;

				CAT_set_sprite_colour(CAT_BLACK);
				CAT_draw_sprite
				(
					sprite, child->toggle_data.toggle,
					CAT_get_text_box_cursor_x(),
					CAT_get_text_box_cursor_y() + CAT_GLYPH_HEIGHT/2 - sprite->height/2
				);
				CAT_text_box_shift_cursor(sprite->width + 4, 0);
			}
			break;

			case CAT_GUI_MENU_TYPE_TICKER:
			{
				CAT_text_box_draw(1, CAT_BLACK, "< %d > ", child->ticker_data.value);
			}
			break;

			default:
			break;
		}

		if(i == head->selector)
			CAT_text_box_draw(1, CAT_BLACK, "<");
		
		CAT_text_box_newline(1);
		CAT_text_box_shift_cursor(0, 4);
	}

	if(CAT_is_last_render_cycle())
	{
		menu_stack_length = 0;
		menu_root = -1;
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
// NOTIF

typedef enum
{
	CAT_GUI_NODE_HEAD_TEXT,
	CAT_GUI_NODE_HEAD_IMAGE,
	CAT_GUI_NODE_HEAD_OPTION
} CAT_gui_node_head;

typedef struct __attribute__((__packed__))
{
	CAT_gui_node_head head;
	char text[128];
} CAT_gui_node_text;

typedef struct __attribute__((__packed__))
{
	CAT_gui_node_head head;
	const CAT_sprite* sprite;
	int frame_idx;
} CAT_gui_node_image;

typedef struct __attribute__((__packed__))
{
	CAT_gui_node_head head;
	char text[24];
	void (*proc)(void*);
	void* arg;
} CAT_gui_node_option;

static void CAT_gui_node_text_init(CAT_gui_node_head* head, const char* text)
{
	CAT_gui_node_text* node = (CAT_gui_node_text*) head;
	node->head = CAT_GUI_NODE_HEAD_TEXT;
	if(text != NULL)
		strcpy(node->text, text);
}

static void CAT_gui_node_image_init(CAT_gui_node_head* head, const CAT_sprite* sprite, int frame_idx)
{
	CAT_gui_node_image* node = (CAT_gui_node_image*) head;
	node->head = CAT_GUI_NODE_HEAD_IMAGE;
	node->sprite = sprite;
	node->frame_idx = frame_idx;
}

static void CAT_gui_node_option_init(CAT_gui_node_head* head, const char* text, void (*proc)(void*), void* arg)
{
	CAT_gui_node_option* node = (CAT_gui_node_option*) head;
	node->head = CAT_GUI_NODE_HEAD_OPTION;
	if(text != NULL)
		strcpy(node->text, text);
	node->proc = proc;
	node->arg = arg;
}

#define NOTIF_NODE_CAPACITY 8

typedef struct
{
	int id;
	enum {
		NOTIF_STATUS_CLOSED,
		NOTIF_STATUS_OPEN,
		NOTIF_STATUS_SHOULD_CLOSE
	} status;
	bool block;

	int x0;
	int y0;
	int x1;
	int y1;

	uint8_t node_arena[NOTIF_NODE_CAPACITY * sizeof(CAT_gui_node_text)];
	CAT_bump_allocator node_balloc;
	CAT_gui_node_head* nodes[NOTIF_NODE_CAPACITY];
	int node_count;
	int selector;
} notif_window;

#define NOTIF_WINDOW_CAPACITY 4
static notif_window notif_windows[NOTIF_WINDOW_CAPACITY];
static int notif_window_count = 0;

#define NOTIF_HANDLE_VALID(window) (window != NULL && window->status != NOTIF_STATUS_CLOSED)

CAT_gui_node_head* gui_notif_add_node(notif_window* window, int size)
{
	if(window->node_count >= NOTIF_NODE_CAPACITY)
		return NULL;

	CAT_gui_node_head* ptr = (CAT_gui_node_head*) CAT_balloc(&window->node_balloc, size);
	window->nodes[window->node_count] = ptr;
	window->node_count += 1;
	return ptr;
}

CAT_gui_handle CAT_gui_notif_open(int x0, int y0, int x1, int y1)
{
	if(notif_window_count >= NOTIF_WINDOW_CAPACITY)
		return NULL;
	notif_window* window = NULL;
	for(int i = 0; i < NOTIF_NODE_CAPACITY; i++)
	{
		if(notif_windows[i].status == NOTIF_STATUS_CLOSED)
		{
			window = &notif_windows[i];
			window->id = i;
			notif_window_count += 1;
			break;
		}
	}

	window->status = NOTIF_STATUS_OPEN;
	window->block = false;

	window->x0 = x0;
	window->y0 = y0;
	window->x1 = x1;
	window->y1 = y1;

	window->node_balloc = CAT_BALLOC_INIT(window->node_arena, sizeof(window->node_arena));
	window->node_count = 0;
	window->selector = 0;

	return (CAT_gui_handle) window;
}

bool CAT_gui_notif_is_open(CAT_gui_handle handle)
{
	notif_window* window = (notif_window*) handle;
	return NOTIF_HANDLE_VALID(window);
}

void CAT_gui_notif_set_block(CAT_gui_handle handle, bool value)
{
	notif_window* window = (notif_window*) handle;
	if(!NOTIF_HANDLE_VALID(window))
		return;

	window->block = value;
}

void CAT_gui_notif_close(CAT_gui_handle handle)
{
	notif_window* window = (notif_window*) handle;
	if(!NOTIF_HANDLE_VALID(window))
		return;

	window->status = NOTIF_STATUS_SHOULD_CLOSE;
}

void CAT_gui_notif_clear(CAT_gui_handle handle)
{
	notif_window* window = (notif_window*) handle;
	if(!NOTIF_HANDLE_VALID(window))
		return;

	CAT_bfree(&window->node_balloc);
	window->node_count = 0;
	window->selector = 0;
}

void CAT_gui_notif_text(CAT_gui_handle handle, const char* fmt, ...)
{
	notif_window* window = (notif_window*) handle;
	if(!NOTIF_HANDLE_VALID(window))
		return;

	CAT_gui_node_head* head = gui_notif_add_node(window, sizeof(CAT_gui_node_text));
	va_list args;
	va_start(args, fmt);
	vsnprintf(((CAT_gui_node_text*) head)->text, 128, fmt, args);
	va_end(args);
	CAT_gui_node_text_init(head, NULL);
}

void CAT_gui_notif_image(CAT_gui_handle handle, const CAT_sprite* sprite, int frame_idx)
{
	notif_window* window = (notif_window*) handle;
	if(!NOTIF_HANDLE_VALID(window))
		return;

	CAT_gui_node_head* head = gui_notif_add_node(window, sizeof(CAT_gui_node_image));
	CAT_gui_node_image_init(head, sprite, frame_idx);
}

void CAT_gui_notif_option(CAT_gui_handle handle, void (*proc)(void*), void* arg, const char* fmt, ...)
{
	notif_window* window = (notif_window*) handle;
	if(!NOTIF_HANDLE_VALID(window))
		return;

	CAT_gui_node_head* head = gui_notif_add_node(window, sizeof(CAT_gui_node_text));
	va_list args;
	va_start(args, fmt);
	vsnprintf(((CAT_gui_node_option*) head)->text, 24, fmt, args);
	va_end(args);
	CAT_gui_node_option_init(head, NULL, proc, NULL);
}

CAT_gui_node_option* notif_get_option(notif_window* window, int idx)
{
	if(!NOTIF_HANDLE_VALID(window))
		return NULL;

	int option_count = 0;
	for(int i = 0; i < window->node_count; i++)
	{
		CAT_gui_node_head* head = window->nodes[i];
		if(*head == CAT_GUI_NODE_HEAD_OPTION)
		{
			if(option_count == idx)
				return (CAT_gui_node_option*) head;
			option_count += 1;
		}
	}
	return NULL;
}

bool notif_is_option_selected(notif_window* window, int idx)
{
	if(!NOTIF_HANDLE_VALID(window))
		return false;

	int option_count = 0;
	for(int i = 0; i < window->node_count; i++)
	{
		CAT_gui_node_head* head = window->nodes[i];
		if(*head == CAT_GUI_NODE_HEAD_OPTION)
		{
			if(i == idx && option_count == window->selector)
				return true;
			option_count += 1;
		}
	}
	return false;
}

void notif_logic(notif_window* window)
{
	if(!NOTIF_HANDLE_VALID(window))
		return;

	int option_count = 0;
	for(int i = 0; i < window->node_count; i++)
	{
		CAT_gui_node_head* head = window->nodes[i];
		if(*head == CAT_GUI_NODE_HEAD_OPTION)
			option_count += 1;
	}

	if(CAT_input_pressed(CAT_BUTTON_UP))
		window->selector -= 1;
	if(CAT_input_pressed(CAT_BUTTON_DOWN))
		window->selector += 1;
	if(window->selector >= option_count)
		window->selector = option_count-1;
	else if(window->selector < 0 && option_count > 0)
		window->selector = 0;

	if(CAT_input_pressed(CAT_BUTTON_A))
	{
		CAT_gui_node_option* option = notif_get_option(window, window->selector);
		if(option != NULL && option->proc != NULL)
			option->proc(option->arg);
	}

	if(window->status == NOTIF_STATUS_SHOULD_CLOSE)
	{
		window->status = NOTIF_STATUS_CLOSED;
		notif_window_count -= 1;
	}
}

void CAT_gui_notif_logic()
{
	for(int i = 0; i < notif_window_count; i++)
	{
		notif_window* window = &notif_windows[i];
		if(NOTIF_HANDLE_VALID(window))
		{
			notif_logic(window);
			if(window->block)
				return;
		}
	}
}

int notif_calculate_y1(notif_window* window)
{
	int y1 = window->y0;

	for(int i = 0; i < window->node_count; i++)
	{
		CAT_gui_node_head* head = window->nodes[i];
		switch (*head)
		{
			case CAT_GUI_NODE_HEAD_TEXT:
			{
				CAT_gui_node_text* node = (CAT_gui_node_text*) head;
				int w, h;
				CAT_TXTAN_measure
				(
					window->x0, window->y0, window->x1, CAT_LCD_SCREEN_H,
					1, node->text,
					&w, &h
				);
				y1 += h + CAT_TEXT_LINE_HEIGHT;
			}
			break;

			case CAT_GUI_NODE_HEAD_IMAGE:
			{
				CAT_gui_node_image* node = (CAT_gui_node_image*) head;
				y1 += node->sprite->height + CAT_LEADING;
			}
			break;

			case CAT_GUI_NODE_HEAD_OPTION:
			{
				CAT_gui_node_option* node = (CAT_gui_node_option*) head;
				int w, h;
				CAT_TXTAN_measure
				(
					window->x0, window->y0, window->x1, CAT_LCD_SCREEN_H,
					1, node->text,
					&w, &h
				);
				y1 += h + CAT_TEXT_LINE_HEIGHT;
			}
			break;
		}
	}

	return y1;
}

void notif_draw(notif_window* window)
{
	int y1 = window->y1 == window->y0 ? notif_calculate_y1(window) + 8 : window->y1;
	int h = y1 - window->y0;

	CAT_fillberry(window->x0, window->y0, window->x1-window->x0, h, CAT_WHITE);
	CAT_strokeberry(window->x0-1, window->y0-1, window->x1-window->x0+2, h+2, CAT_BLACK);
	CAT_strokeberry(window->x0-2, window->y0-2, window->x1-window->x0+4, h+4, CAT_GREY);

	CAT_set_text_box(window->x0+4, window->y0+4, window->x1-4, y1-4);
	for(int i = 0; i < window->node_count; i++)
	{
		CAT_gui_node_head* head = window->nodes[i];
		switch (*head)
		{
			case CAT_GUI_NODE_HEAD_TEXT:
			{
				CAT_gui_node_text* node = (CAT_gui_node_text*) head;
				CAT_text_box_draw(1, CAT_BLACK, node->text);
				CAT_text_box_newline(1);
			}
			break;

			case CAT_GUI_NODE_HEAD_IMAGE:
			{
				CAT_gui_node_image* node = (CAT_gui_node_image*) head;
				CAT_draw_sprite
				(
					node->sprite, node->frame_idx,
					CAT_get_text_box_cursor_x(),
					CAT_get_text_box_cursor_y()
				);
				CAT_text_box_shift_cursor(0, node->sprite->height + CAT_LEADING);
			}
			break;

			case CAT_GUI_NODE_HEAD_OPTION:
			{
				CAT_gui_node_option* node = (CAT_gui_node_option*) head;
				bool selected = notif_is_option_selected(window, i);
				if(selected)
					CAT_text_box_draw(1, CAT_BLACK, "[%s]", node->text);
				else
					CAT_text_box_draw(1, CAT_BLACK, "[%s]", node->text);
				CAT_text_box_newline(1);
			}
			break;
		}
	};
}

void CAT_gui_notif()
{
	for(int i = 0; i < notif_window_count; i++)
	{
		notif_window* window = &notif_windows[i];
		if(window->status != NOTIF_STATUS_CLOSED)
			notif_draw(window);
	}
}


//////////////////////////////////////////////////////////////////////////
// IT'S ABOUT TO GET WEIRD IN HERE

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
		GUI_NODE_TYPE_OPTION
	} type;
	bool hot;

	// PAYLOAD
	union
	{
		struct // : WINDOW
		{
			int x0, y0, x1, y1;
			GUI_node* children[32];
			int child_count;
			int selector;
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

static void GUI_pool_free_transient()
{
	for(int i = 0; i < GUI_POOL_CAPACITY; i++)
	{
		if(GUI_pool[i].id == GUI_ID_NULL)
			GUI_pool[i].parent = GUI_ID_NULL;
	}
}

static GUI_node* GUI_pool_get(GUI_ID id)
{
	for(int i = 0; i < GUI_POOL_CAPACITY; i++)
	{
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

static GUI_node* GUI_pool_register(GUI_node* node, bool persistent)
{
	GUI_node* destination = NULL;
	if(persistent)
		destination = GUI_pool_get(node->id);
	if(destination == NULL)
		destination = GUI_pool_get_free();
	if(destination != NULL)
		memcpy(destination, node, sizeof(GUI_node));
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
	stack->data[idx] = id;
	stack->count += 1;
}

static GUI_ID GUI_ID_stack_pop(GUI_ID_stack* stack)
{
	if(stack->count <= 0)
		return GUI_ID_NULL;
	stack->count -= 1;
	GUI_ID back = stack->data[stack->count];
	return back;
}

static GUI_ID GUI_ID_stack_get(GUI_ID_stack* stack, int idx)
{
	if(idx < 0)
		idx += stack->count;
	if(idx >= stack->count)
		return GUI_ID_NULL;
	return stack->data[idx];
}

static GUI_ID_stack transient_stack;
static GUI_ID_stack persistent_stack;

void CAT_GUI_push_window(const char* text)
{
	GUI_ID parent = GUI_ID_stack_get(&transient_stack, -1);
	GUI_ID id = CAT_CRC32_hash(text, parent);
	GUI_ID_stack_push(&transient_stack, id);
	GUI_ID_stack_push(&persistent_stack, id);
}

void CAT_GUI_pop_window()
{
	GUI_ID_stack_pop(&persistent_stack);
}

bool CAT_GUI_begin_window(const char* text, int x0, int y0, int x1, int y1)
{
	GUI_ID parent = GUI_ID_stack_get(&transient_stack, -2);
	GUI_ID id = CAT_CRC32_hash(text, parent);
	if(GUI_ID_stack_get(&transient_stack, -1) != id)
		return false;
	GUI_node values =
	{
		.parent = parent,
		.id = id,
		.type = GUI_NODE_TYPE_WINDOW,

		.x0 = x0,
		.y0 = y0,
		.x1 = x1,
		.y1 = y1,

		.child_count = 0
	};
	GUI_node* node = GUI_pool_register(&values, true);
	return true;
}

void CAT_GUI_end_window()
{
	GUI_ID_stack_pop(&transient_stack);
}

void CAT_GUI_text(const char* text)
{
	GUI_ID parent_id = GUI_ID_stack_get(&transient_stack, -1);
	GUI_node* parent_node = GUI_pool_get(parent_id);

	GUI_node values =
	{
		.parent = parent_id,
		.type = GUI_NODE_TYPE_TEXT,
	};
	strcpy(values.text, text);
	GUI_node* node = GUI_pool_register(&values, false);
	GUI_node_add_child(parent_node, node);
}

bool CAT_GUI_option(const char* text)
{
	GUI_ID parent_id = GUI_ID_stack_get(&transient_stack, -1);
	GUI_node* parent_node = GUI_pool_get(parent_id);

	GUI_ID id = CAT_CRC32_hash(text, parent_id);
	GUI_node* existing = GUI_pool_get(id);

	bool result = false;
	if(existing != NULL)
		result = existing->option.trigger;

	GUI_node values =
	{
		.parent = parent_id,
		.id = id,
		.type = GUI_NODE_TYPE_OPTION,
	};
	strcpy(values.option.text, text);
	GUI_node* node = GUI_pool_register(&values, true);
	GUI_node_add_child(parent_node, node);

	return result;
}

void CAT_GUI_new_frame()
{
	GUI_pool_free_transient();
	GUI_ID_stack_clear(&transient_stack);
	GUI_ID_stack_clear(&persistent_stack);
}

static int GUI_count_selectables(GUI_node* node)
{
	int selectable_count = 0;
	for(int i = 0; i < node->child_count; i++)
	{
		if(node->children[i]->type == GUI_NODE_TYPE_OPTION)
			selectable_count += 1;
	}
	return selectable_count;
}

static GUI_node* GUI_get_selectable(GUI_node* node, int idx)
{
	int selectable_count = 0;
	for(int i = 0; i < node->child_count; i++)
	{
		if(node->children[i]->type == GUI_NODE_TYPE_OPTION)
		{
			if(selectable_count == idx)
				return node->children[i];
			selectable_count += 1;
		}
	}
	return NULL;
}

void CAT_GUI_IO()
{
	for(int i = 0; i < persistent_stack.count; i++)
	{
		GUI_ID id = persistent_stack.data[i];
		GUI_node* node = GUI_pool_get(id);
		node->hot = true;

		for(int j = 0; j < node->child_count; j++)
		{
			GUI_node* child = node->children[j];
			child->hot = true;
		}		
	}
	for(int i = 0; i < GUI_POOL_CAPACITY; i++)
	{
		if(!GUI_pool[i].hot)
			GUI_pool[i].parent = GUI_ID_NULL;
	}

	GUI_ID head_id = GUI_ID_stack_get(&persistent_stack, -1);
	GUI_node* head = GUI_pool_get(head_id);

	int selectable_count = GUI_count_selectables(head);
	if(selectable_count > 0)
	{
		if(CAT_input_pressed(CAT_BUTTON_UP))
			head->selector -= 1;
		if(CAT_input_pressed(CAT_BUTTON_DOWN))
			head->selector += 1;
		head->selector = CAT_clamp(head->selector, 0, selectable_count-1);

		if(CAT_input_pressed(CAT_BUTTON_A))
		{
			GUI_node* selected = GUI_get_selectable(head, head->selector);
			if(selected != NULL)
				selected->option.trigger = true;
		}
	}
	else
		head->selector = -1;
}

void GUI_draw_window(GUI_node* node)
{
	CAT_fillberry(node->x0, node->y0, node->x1-node->x0, node->y1-node->y0, CAT_WHITE);
	CAT_lineberry(node->x0-1, node->y0-1, node->x0-1, node->y1+1, CAT_GREY);
	CAT_lineberry(node->x0-1, node->y0-1, node->x1+1, node->y0-1, CAT_GREY);
	CAT_lineberry(node->x1+1, node->y0, node->x1+1, node->y1+1, CAT_BLACK);
	CAT_lineberry(node->x0, node->y1+1, node->x1, node->y1+1, CAT_BLACK);
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
		CAT_text_box_draw(1, CAT_BLACK, "\1 [%s]", node->option.text);
	else
		CAT_text_box_draw(1, CAT_BLACK, "\1 %s", node->option.text);
	CAT_text_box_newline(1);
}

void CAT_GUI_draw()
{
	for(int i = 0; i < persistent_stack.count; i++)
	{
		GUI_ID id = persistent_stack.data[i];
		GUI_node* node = GUI_pool_get(id);
		GUI_draw_window(node);

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

				default:
				{
					CAT_text_box_draw(1, CAT_BLACK, "<UNKNOWN>");
					CAT_text_box_newline(1);
				}
				break;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// FINALIZATION

void CAT_gui_tick()
{
	CAT_gui_notif_logic();
	
	if(CAT_gui_popup_is_open())
	{
		CAT_gui_popup_logic();
		return;
	}

	if(CAT_gui_keyboard_is_open())
	{
		CAT_gui_keyboard_logic();
		return;
	}

	if(CAT_gui_item_grid_is_open())
	{
		CAT_gui_item_grid_logic();
		return;
	}

	if(CAT_gui_menu_is_open())
	{
		CAT_gui_menu_logic();
		return;
	}
}

void CAT_gui_render()
{
	if(CAT_gui_menu_is_open())
		CAT_gui_menu();
	else if(CAT_gui_item_grid_is_open())
		CAT_gui_item_grid();

	if(CAT_gui_keyboard_is_open())
		CAT_gui_keyboard();
	if(CAT_gui_popup_is_open())
		CAT_gui_popup();
	CAT_gui_notif();
}