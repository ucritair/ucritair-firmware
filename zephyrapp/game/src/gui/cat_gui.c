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

//////////////////////////////////////////////////////////////////////////
// BASICS (DEPRECATED)

typedef enum CAT_gui_flag
{
	CAT_GUI_FLAG_NONE = 0,
	CAT_GUI_FLAG_BORDERED = (1 << 0),
	CAT_GUI_FLAG_TIGHT = (1 << 1),
	CAT_GUI_FLAG_WRAPPED = (1 << 2),
} CAT_gui_flag;

typedef struct CAT_gui
{
	CAT_gui_flag flags;

	CAT_ivec2 start;
	CAT_ivec2 shape;
	CAT_ivec2 cursor;

	int margin;
	int pad;
	int channel_height;
} CAT_gui;
extern CAT_gui gui;

CAT_gui gui =
{
	.flags = CAT_GUI_FLAG_NONE,

	.start = (CAT_ivec2) {0, 0},
	.shape = (CAT_ivec2) {0, 0},
	.cursor = (CAT_ivec2) {0, 0},

	.margin = 8,
	.pad = 4,
	.channel_height = 0
};

void CAT_gui_set_flag(int flag)
{
	gui.flags |= flag;
}

bool CAT_gui_consume_flag(int flag)
{
	bool value = gui.flags & flag;
	gui.flags &= ~flag;
	return value;
}

int CAT_gui_clear_flags()
{
	int value = gui.flags;
	gui.flags = CAT_GUI_FLAG_NONE;
	return value;
}

void CAT_gui_panel(CAT_ivec2 start, CAT_ivec2 shape)
{
	CAT_fillberry(start.x * CAT_TILE_SIZE, start.y * CAT_TILE_SIZE, shape.x * CAT_TILE_SIZE, shape.y * CAT_TILE_SIZE, 0xFFFF);
	if(CAT_gui_consume_flag(CAT_GUI_FLAG_BORDERED))
		CAT_strokeberry(start.x * CAT_TILE_SIZE, start.y * CAT_TILE_SIZE, shape.x * CAT_TILE_SIZE, shape.y * CAT_TILE_SIZE, 0x0000);

	gui.start = start;
	gui.shape = shape;
	gui.cursor = CAT_ivec2_mul(start, CAT_TILE_SIZE);
	gui.cursor.y += gui.margin;
	gui.cursor.x += gui.margin;
	if(CAT_gui_consume_flag(CAT_GUI_FLAG_TIGHT))
	{
		gui.cursor.y -= gui.margin/2;
		gui.cursor.x -= gui.margin/2;
	}
	gui.channel_height = 0;
}

void gui_open_channel(int height)
{
	if(gui.channel_height == 0)
		gui.cursor.y += height / 2;
	if(height > gui.channel_height)
		gui.channel_height = height;
}

void CAT_gui_line_break()
{
	gui.cursor.y +=
	(gui.channel_height > 0 ?
	gui.channel_height / 2 :
	CAT_GLYPH_HEIGHT) +
	gui.pad;
	gui.cursor.x = gui.start.x * CAT_TILE_SIZE + gui.margin;
	gui.channel_height = 0;
}

void CAT_gui_text(const char* text)
{
	bool wrap = CAT_gui_consume_flag(CAT_GUI_FLAG_WRAPPED);
	int x_lim = (gui.start.x * CAT_TILE_SIZE) + (gui.shape.x) * CAT_TILE_SIZE - CAT_GLYPH_WIDTH - gui.margin;
	const char* c = text;

	while
	(
		*c != '\0' &&
		!(*c == '#' && *(c+1) == '#')
	)
	{
		if(wrap && gui.cursor.x >= x_lim && !isspace(*(c+1)))
		{
			if(!isspace(*c) && !isspace(*(c-1)))
			{
				CAT_set_sprite_flags(CAT_DRAW_FLAG_CENTER_Y);
				CAT_draw_sprite(&glyph_sprite, '-', gui.cursor.x, gui.cursor.y);
			}
			CAT_gui_line_break();	
			if(isspace(*c))
				c++;
		}

		if(*c == '\n')
		{
			CAT_gui_line_break();
			c++;
			continue;
		}

		gui_open_channel(CAT_GLYPH_HEIGHT);
		CAT_set_sprite_flags(CAT_DRAW_FLAG_CENTER_Y);
		CAT_draw_sprite(&glyph_sprite, *c, gui.cursor.x, gui.cursor.y);
		gui.cursor.x += CAT_GLYPH_WIDTH;
		c++;
	}
}

void CAT_gui_image(const CAT_sprite* sprite, int frame_idx)
{
	gui_open_channel(sprite->height);

	gui.cursor.x += gui.pad / 2;
	CAT_set_sprite_flags(CAT_DRAW_FLAG_CENTER_Y);
	CAT_draw_sprite(sprite, frame_idx, gui.cursor.x, gui.cursor.y);
	gui.cursor.x += sprite->width;
	gui.cursor.x += gui.pad / 2;
}

void CAT_gui_div(const char* text)
{
	CAT_gui_line_break();
	gui_open_channel(CAT_TILE_SIZE);
	if(strlen(text) == 0)
	{
		CAT_lineberry(0, gui.cursor.y, CAT_LCD_SCREEN_W, gui.cursor.y, 0x0000);
	}
	else
	{
		CAT_gui_text(text);
		int start = gui.cursor.x + gui.pad;
		CAT_lineberry(start, gui.cursor.y, CAT_LCD_SCREEN_W-gui.margin, gui.cursor.y, 0x0000);
	}
	CAT_gui_line_break();
}

void CAT_gui_textf(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	char text[256];
	vsnprintf(text, 256, fmt, args);
	va_end(args);

	CAT_gui_text(text);
}

void CAT_gui_title(bool tabs, const char* fmt, ...)
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});
	CAT_lineberry(0, 31, CAT_LCD_SCREEN_W, 31, 0x0000);
	
	if(tabs)
		CAT_gui_text("< ");

	va_list args;
	va_start(args, fmt);
	char text[32];
	vsnprintf(text, 32, fmt, args);
	va_end(args);
	CAT_gui_text(text);

	if(tabs)
		CAT_gui_text(" >");
	CAT_gui_text(" ");
}


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
	CAT_draw_textf(KEYB_PAD - overshoot * CAT_GLYPH_WIDTH, cursor_y, "%s%s", keyb_buffer, keyb_show_cursor ? "|" : "");
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

		CAT_draw_text(cursor_x+KEYB_PAD/2, cursor_y+KEYB_PAD/2, keyb_buttons[i].title);

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
	CAT_gui_panel((CAT_ivec2) {POPUP_TILE_X, POPUP_TILE_Y}, (CAT_ivec2) {POPUP_TILE_W, POPUP_TILE_H});
	CAT_strokeberry(POPUP_X, POPUP_Y, POPUP_W, POPUP_H, CAT_BLACK);
	CAT_strokeberry(POPUP_X-1, POPUP_Y-1, POPUP_W+2, POPUP_H+2, CAT_GREY);
	
	CAT_set_text_mask(POPUP_X+POPUP_MARGIN, -1, POPUP_X+POPUP_W-POPUP_MARGIN, -1);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	CAT_draw_text(POPUP_X+POPUP_MARGIN, POPUP_Y+POPUP_MARGIN, popup_text);

	const char* responses =
	popup_style == CAT_POPUP_STYLE_YES_NO ?
	(popup_selector == 0 ? "[YES]   NO \n" : " YES   [NO]\n") :
	"[OKAY]";

	CAT_draw_text
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

void CAT_gui_menu()
{
	menu_node* head = get_global_head();

	CAT_gui_title
	(
		false,
		head->title
	);

	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

	for(int i = 0; i < head->child_count; i++)
	{
		menu_node* child = &menu_table[head->children[i]];

		if(child->type == CAT_GUI_MENU_TYPE_TEXT)
			CAT_gui_textf("%s ", child->title);
		else
			CAT_gui_textf("\1 %s ", child->title);

		switch (child->type)
		{
			case CAT_GUI_MENU_TYPE_TOGGLE:
			{
				CAT_set_sprite_colour(CAT_BLACK);
				const CAT_sprite* sprite =
				child->toggle_data.style == CAT_GUI_TOGGLE_STYLE_CHECKBOX ?
				&ui_checkbox_sprite : &ui_radio_button_circle_sprite;
				CAT_gui_image(sprite, child->toggle_data.toggle);
				CAT_gui_text(" ");
			}
			break;

			case CAT_GUI_MENU_TYPE_TICKER:
			{
				CAT_gui_textf("< %d > ", child->ticker_data.value);
			}
			break;

			default:
			break;
		}

		if(i == head->selector)
			CAT_gui_text("<");
			
		CAT_gui_line_break();
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

	CAT_set_text_colour(CAT_WHITE);
	CAT_set_text_mask(4, 4, CAT_LCD_SCREEN_W-4, 4+26);
	const char* item_name = !item_grid_is_empty() ? item_table.data[item_grid_pool.data[item_grid_selector]].name : "";
	if(item_grid_tab_count > 1)
		CAT_draw_textf(4, 8, ">>> %s/%s", tab->title, item_name);
	else
		CAT_draw_textf(4, 8, ">>> %s", item_name);
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
		CAT_set_text_colour(CAT_WHITE);
		CAT_draw_text(4, y, "There's nothing here...");
	}

	if (item_grid_get_page() < (item_grid_pool.length-ITEM_GRID_CELLS))
		CAT_draw_sprite(&ui_down_arrow_sprite, -1, 240 - 32, 320 - 24);

	if(item_grid_focus == NAV)
	{
		CAT_fillberry(ITEM_GRID_NAV_X, ITEM_GRID_NAV_Y, ITEM_GRID_NAV_W, ITEM_GRID_NAV_H, ITEM_GRID_NAV_COLOUR);
		CAT_lineberry(ITEM_GRID_NAV_X, ITEM_GRID_NAV_Y, ITEM_GRID_NAV_X+ITEM_GRID_NAV_W, ITEM_GRID_NAV_Y, CAT_WHITE);
		CAT_lineberry(ITEM_GRID_NAV_X, ITEM_GRID_NAV_Y+ITEM_GRID_NAV_H, ITEM_GRID_NAV_X+ITEM_GRID_NAV_W, ITEM_GRID_NAV_Y+ITEM_GRID_NAV_H, CAT_WHITE);

		CAT_set_text_colour(CAT_WHITE);
		CAT_set_text_scale(2);
		CAT_set_text_flags(CAT_TEXT_FLAG_CENTER);
		CAT_draw_text(CAT_LCD_SCREEN_W/2, ITEM_GRID_NAV_Y + ITEM_GRID_NAV_H/2 - 12, tab->title);
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
// DIALOGUE BOX

const char* dialogue_text = "";
int dialogue_duration = 0;
uint64_t dialogue_timestamp = 0;
bool dialogue_open = false;

#define DIALOGUE_X 4
#define DIALOGUE_Y 4
#define DIALOGUE_W CAT_LCD_SCREEN_W-8
#define DIALOGUE_H (CAT_LCD_SCREEN_H / 6)

void CAT_gui_open_dialogue(const char* text, int duration)
{
	dialogue_text = text;
	dialogue_duration = duration;
	dialogue_timestamp = CAT_get_RTC_now();
	dialogue_open = true;
}

bool CAT_gui_dialogue_is_open()
{
	return dialogue_open;
}

void CAT_gui_dismiss_dialogue()
{
	dialogue_open = false;
}

void CAT_gui_dialogue_logic()
{
	if((CAT_get_RTC_now() - dialogue_timestamp) > dialogue_duration)
		dialogue_open = false;
}

void CAT_gui_dialogue()
{
	CAT_fillberry(DIALOGUE_X, DIALOGUE_Y, DIALOGUE_W, DIALOGUE_H, CAT_WHITE);
	CAT_strokeberry(DIALOGUE_X, DIALOGUE_Y, DIALOGUE_W, DIALOGUE_H, CAT_GREY);
	CAT_strokeberry(DIALOGUE_X+1, DIALOGUE_Y+1, DIALOGUE_W-2, DIALOGUE_H-2, CAT_GREY);
	
	CAT_set_text_mask(DIALOGUE_X+4, -1, DIALOGUE_X+DIALOGUE_W-4, -1);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	CAT_draw_text(DIALOGUE_X+4, DIALOGUE_X+4, dialogue_text);
}


//////////////////////////////////////////////////////////////////////////
// FINALIZATION

void CAT_gui_tick()
{
	if(CAT_gui_popup_is_open())
		CAT_gui_popup_logic();
	else if(CAT_gui_keyboard_is_open())
		CAT_gui_keyboard_logic();
	else if(CAT_gui_item_grid_is_open())
		CAT_gui_item_grid_logic();
	else if(CAT_gui_menu_is_open())
		CAT_gui_menu_logic();

	if(CAT_gui_dialogue_is_open())
		CAT_gui_dialogue_logic();
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
	if(CAT_gui_dialogue_is_open())
		CAT_gui_dialogue();	
}