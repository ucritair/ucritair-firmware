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

//////////////////////////////////////////////////////////////////////////
// BASICS

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
	char text[512];
	vsnprintf(text, 512, fmt, args);
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
		"0123456789\x06",
		"QWERTYUIOP",
		"ASDFGHJKL",
		"ZXCVBNM_\x08"
	},
	(const char*[]) {
		"0123456789\x06",
		"qwertyuiop",
		"asdfghjkl",
		"zxcvbnm_\x08"
	}
};

static struct 
{
	bool open;
	char* target;

	char buffer[32];
	int cursor;
	
	bool show_cursor;
	float cursor_timer;

	int case_idx;
	int row_idx;
	int glyph_idx;
} keyboard = 
{
	.open = false,
	.target = NULL,
	.cursor = 0,
	.show_cursor = true,
	.cursor_timer = 0.0f,
	.case_idx = 0,
	.row_idx = 0,
	.glyph_idx = 0
};

void CAT_gui_open_keyboard(char* target)
{
	if(keyboard.open)
		return;

	keyboard.open = true;
	keyboard.target = target;
	int length = strlen(target);
	strncpy(keyboard.buffer, target, 32);
	keyboard.cursor = length;
	keyboard.case_idx = 0;
	keyboard.row_idx = 0;
	keyboard.glyph_idx = 0;

	CAT_input_clear();
}

static void gui_close_keyboard()
{
	keyboard.open = false;
}

bool CAT_gui_keyboard_is_open()
{
	return keyboard.open;
}

void CAT_gui_keyboard_io()
{
	if(CAT_input_pressed(CAT_BUTTON_B))
		gui_close_keyboard();
	
	const char** typecase = typecases[keyboard.case_idx];
	if(CAT_input_pulse(CAT_BUTTON_UP))
		keyboard.row_idx -= 1;
	if(CAT_input_pulse(CAT_BUTTON_DOWN))
		keyboard.row_idx += 1;
	keyboard.row_idx = clamp(keyboard.row_idx, 0, 4);

	if(keyboard.row_idx >= 4)
	{
		if(CAT_input_pressed(CAT_BUTTON_A))
		{
			if(keyboard.target != NULL)
			{
				strcpy(keyboard.target, keyboard.buffer);
			}
			gui_close_keyboard();
		}
		return;
	}

	const char* row = typecase[keyboard.row_idx];
	if(CAT_input_pulse(CAT_BUTTON_RIGHT))
		keyboard.glyph_idx += 1;
	if(CAT_input_pulse(CAT_BUTTON_LEFT))
		keyboard.glyph_idx -= 1;
	keyboard.glyph_idx = clamp(keyboard.glyph_idx, 0, strlen(row)-1);

	if(CAT_input_pressed(CAT_BUTTON_A))
	{
		char glyph = row[keyboard.glyph_idx];
		if(glyph == 6)
		{
			if(keyboard.cursor > 0)
			{
				keyboard.cursor -= 1;
				keyboard.show_cursor = true;
				keyboard.cursor_timer = 0;
			}
		}
		else if(glyph == 8)
			keyboard.case_idx = !keyboard.case_idx;
		else 
		{
			glyph = (glyph == '_') ? ' ' : glyph;
			if(keyboard.cursor < CAT_TEXT_INPUT_MAX_LENGTH)
			{
				keyboard.buffer[keyboard.cursor] = glyph;
				keyboard.cursor += 1;
			}
		}
		keyboard.buffer[keyboard.cursor] = '\0';
	}

	keyboard.cursor_timer += CAT_get_delta_time_s();
	if(keyboard.cursor_timer >= 0.5f)
	{
		keyboard.cursor_timer = 0.0f;
		keyboard.show_cursor = !keyboard.show_cursor;
	}
}

void CAT_gui_keyboard()
{	
	CAT_gui_panel((CAT_ivec2){0, 10}, (CAT_ivec2){15, 10});
	CAT_lineberry(0, 160, CAT_LCD_SCREEN_W, 160, 0x0000);
	CAT_gui_text(keyboard.buffer);
	if(keyboard.show_cursor)
		CAT_gui_text("|");
	gui.cursor.y -= 4;
	CAT_gui_div("");

	int x_w = gui.margin * 2;
	int y_w = gui.cursor.y;

	const char** typecase = typecases[keyboard.case_idx];
	for(int i = 0; i < 4; i++)
	{
		const char* row = typecase[i];
		for(int j = 0; j < strlen(row); j++)
		{
			char glyph = row[j];
			CAT_draw_sprite(&glyph_sprite, glyph, x_w + 2, y_w + 3);
			if(i == keyboard.row_idx && j == keyboard.glyph_idx)
				CAT_strokeberry(x_w, y_w, CAT_GLYPH_WIDTH + 4, CAT_GLYPH_HEIGHT + 6, 0);
			x_w += CAT_GLYPH_WIDTH + 8;
		}
		y_w += CAT_GLYPH_HEIGHT + 8;
		x_w = gui.margin * 2;
	}
	
	gui.cursor = (CAT_ivec2){x_w, y_w + 2};
	CAT_gui_text("SAVE");
	if(keyboard.row_idx >= 4)
		CAT_strokeberry(x_w - 2, y_w, 4 * CAT_GLYPH_WIDTH + 4, CAT_GLYPH_HEIGHT + 4, 0x0000);
}


//////////////////////////////////////////////////////////////////////////
// POPUP

struct
{
	const char* msg;
	bool result;
	uint8_t selector;
	bool open;
} popup =
{
	NULL,
	false,
	0,
	false
};

void CAT_gui_open_popup(const char* msg)
{	
	popup.msg = msg;
	popup.result = false;
	popup.selector = 0;
	popup.open = true;
	
	CAT_input_clear();
}

bool CAT_gui_popup_is_open()
{
	return popup.open;
}

void CAT_gui_popup_io()
{
	if(CAT_input_pressed(CAT_BUTTON_LEFT))
		popup.selector = 1;
	if(CAT_input_pressed(CAT_BUTTON_RIGHT))
		popup.selector = 0;

	if(CAT_input_pressed(CAT_BUTTON_A))
	{
		popup.result = popup.selector;
		popup.open = false;
	}
	if(CAT_input_pressed(CAT_BUTTON_START) || CAT_input_pressed(CAT_BUTTON_B))
	{
		popup.result = false;
		popup.open = false;
	}
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
	CAT_draw_text(POPUP_X+POPUP_MARGIN, POPUP_Y+POPUP_MARGIN, popup.msg);
	CAT_draw_text
	(
		POPUP_X+POPUP_MARGIN, POPUP_Y+POPUP_H-POPUP_MARGIN-CAT_GLYPH_HEIGHT,
		popup.selector ? "[YES]  NO " : " YES  [NO]"
	);
}

bool CAT_gui_consume_popup()
{
	if(popup.open)
		return false;
	bool result = popup.result;
	popup.result = false;
	return result;
}


//////////////////////////////////////////////////////////////////////////
// MENU

unsigned long hash(const char* s)
{
	unsigned long h = 0;
	while (*s != '\0')
	{
		h = (h << 4) + *(s++);
		unsigned long g = h & 0xF0000000L;
		if (g != 0)
			h ^= g >> 24;
		h &= ~g;
	}
	return h;
}

typedef struct menu_node
{
	bool live;

	const char* title;
	bool clicked;
	
	CAT_gui_menu_type type;
	union 
	{
		struct {} default_data;

		struct
		{
			bool toggle;
			int style;
		} toggle_data;
		
		struct
		{
			int* ticker;
			int min;
			int max;
		} ticker_data;

		struct
		{
			char text[32];
		} text_data;
	};
	
	int16_t parent;
	uint16_t children[32];
	uint8_t child_count;
	int8_t selector;
} menu_node;

static menu_node menu_table[512];
#define MENU_TABLE_SIZE (sizeof(menu_table) / sizeof(menu_table[0]))
static uint16_t menu_stack[24];
#define MENU_STACK_SIZE (sizeof(menu_stack) / sizeof(menu_stack[0]))
static uint8_t menu_stack_length = 0;
static int menu_root = -1;

uint16_t register_menu_node(const char* title, CAT_gui_menu_type type)
{
	uint16_t idx = hash(title) % MENU_TABLE_SIZE;
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
		.title = title,
		.clicked = false,

		.type = type,
		.default_data = {},

		.parent = -1,
		.child_count = 0,
		.selector = 0
	};
	return idx;
}

int find_menu_node(const char* title)
{
	uint16_t idx = hash(title) % MENU_TABLE_SIZE;
	while (menu_table[idx].live)
	{
		if (strcmp(menu_table[idx].title, title) == 0)
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


void CAT_gui_begin_menu_context()
{
	for(int i = 0; i < MENU_TABLE_SIZE; i++)
	{
		menu_table[i].clicked = false;
		menu_table[i].parent = -1;
		menu_table[i].child_count = 0;
	}
}

void CAT_gui_clear_menu_context()
{
	for(int i = 0; i < MENU_TABLE_SIZE; i++)
	{
		menu_table[i].selector = 0;
	}
}

bool CAT_gui_begin_menu(const char* title)
{
	int idx = find_menu_node(title);
	if(idx == -1)
		idx = register_menu_node(title, CAT_GUI_MENU_TYPE_DEFAULT);
	menu_table[idx].child_count = 0;

	if (!CAT_gui_menu_is_open())
	{
		menu_root = idx;
		menu_table[idx].clicked = true;
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

bool CAT_gui_menu_is_open()
{
	return menu_root != -1;
}

bool CAT_gui_menu_item(const char* title)
{
	int idx = find_menu_node(title);
	if(idx == -1)
		idx = register_menu_node(title, CAT_GUI_MENU_TYPE_DEFAULT);
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

	node->toggle_data.toggle = toggle;
	node->toggle_data.style = style;

	return consume_click(idx);
}

bool CAT_gui_menu_ticker(const char* title, int* ticker, int min, int max)
{	
	int idx = find_menu_node(title);
	if(idx == -1)
		idx = register_menu_node(title, CAT_GUI_MENU_TYPE_TICKER);
	menu_add_child(idx);
	menu_node* node = &menu_table[idx];

	node->ticker_data.ticker = ticker;
	node->ticker_data.min = min;
	node->ticker_data.max = max;

	return consume_click(idx);
}

bool CAT_gui_menu_text(const char* fmt, ...)
{
	char temp[32];
	va_list args;
	va_start(args, fmt);
	vsnprintf(temp, 32, fmt, args);
	va_end(args);

	int idx = find_menu_node(temp);
	if(idx == -1)
		idx = register_menu_node(temp, CAT_GUI_MENU_TYPE_TEXT);
	menu_add_child(idx);
	menu_node* node = &menu_table[idx];

	strcpy(node->text_data.text, temp);
	node->title = node->text_data.text;

	return consume_click(idx);
}

void CAT_gui_end_menu()
{
	pop_menu_node();
}

void CAT_gui_menu_io()
{
	menu_node* head = get_global_head();
	
	if(CAT_input_pressed(CAT_BUTTON_UP))
		head->selector -= 1;
	if(CAT_input_pressed(CAT_BUTTON_DOWN))
		head->selector += 1;
	head->selector = (head->selector + head->child_count) % head->child_count;
	menu_node* selected = &menu_table[head->children[head->selector]];

	switch (selected->type)
	{
		case CAT_GUI_MENU_TYPE_TICKER:
		{
			int* ticker = selected->ticker_data.ticker;
			if(CAT_input_pulse(CAT_BUTTON_LEFT))
				*ticker -= 1;
			if(CAT_input_pulse(CAT_BUTTON_RIGHT))
				*ticker += 1;
			if(*ticker < selected->ticker_data.min)
				*ticker = selected->ticker_data.max;
			if(*ticker > selected->ticker_data.max)
				*ticker = selected->ticker_data.min;
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
			CAT_machine_back();
		}
		else
			head->clicked = false;
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

		if(child->type != CAT_GUI_MENU_TYPE_TEXT)
			CAT_gui_textf("\1 ");
		CAT_gui_textf("%s ", child->title);

		switch (child->type)
		{
			case CAT_GUI_MENU_TYPE_TOGGLE:
				CAT_set_sprite_colour(CAT_BLACK);
				const CAT_sprite* sprite =
				child->toggle_data.style == CAT_GUI_TOGGLE_STYLE_CHECKBOX ?
				&ui_checkbox_sprite : &ui_radio_button_circle_sprite;
				CAT_gui_image(sprite, child->toggle_data.toggle);
				CAT_gui_text(" ");
			break;
			case CAT_GUI_MENU_TYPE_TICKER:
				CAT_gui_textf("< %d > ", *(int*)(child->ticker_data.ticker));
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
// PRINTING

static int printf_cursor_y = 0;

void CAT_gui_printf(uint16_t colour, const char* fmt, ...)
{	
	va_list args;
	va_start(args, fmt);
	char text[128];
	vsnprintf(text, 128, fmt, args);
	va_end(args);
	
	int modified_y = printf_cursor_y - CAT_LCD_FRAMEBUFFER_OFFSET;
	if(modified_y < 0 || modified_y >= CAT_LCD_FRAMEBUFFER_H)
		return;

	CAT_set_text_colour(colour);
	CAT_draw_text(0, printf_cursor_y, text);
	printf_cursor_y += CAT_GLYPH_HEIGHT + 2;
}


//////////////////////////////////////////////////////////////////////////
// ITEM GRID

#define ITEM_GRID_MARGIN 12
#define ITEM_GRID_CELL_SIZE 64
#define ITEM_GRID_COLS 3
#define ITEM_GRID_SELECT_TIME 1
#define ITEM_GRID_HEADER_HEIGHT 56
#define ITEM_GRID_BG_COLOUR 0xbdb4
#define ITEM_GRID_ACTION_COLOUR RGB8882565(255 - 0, 255 - 141, 255 - 141)

static bool item_grid_status = false;
static const char* item_grid_title;
static CAT_int_list* item_grid_roster;
static CAT_item_proc item_grid_action;
static int item_grid_flags;

static int item_grid_pool_backing[CAT_ITEM_TABLE_CAPACITY];
static CAT_int_list item_grid_pool;
static int item_grid_selector = -1;
static int item_grid_last_marked = -1;

static int item_grid_anchor_y = 0;
static int item_grid_delta_y = 0;
static bool item_grid_scrolling = false;
static float item_grid_select_timer = 0;

static const char* item_grid_text = "There's nothing here...";

void CAT_gui_begin_item_grid_context()
{
	item_grid_status = false;
	item_grid_title = "";
	item_grid_roster = NULL;
	item_grid_action = NULL;
	item_grid_flags = CAT_GUI_ITEM_GRID_FLAG_NONE;

	CAT_ilist(&item_grid_pool, item_grid_pool_backing, CAT_ITEM_TABLE_CAPACITY);
	item_grid_selector = -1;
	item_grid_last_marked = -1;

	item_grid_anchor_y = 0;
	item_grid_delta_y = 0;
	item_grid_scrolling = false;
	item_grid_select_timer = 0;

	item_grid_text = "There's nothing here...";
}

void CAT_gui_begin_item_grid(const char* title, CAT_int_list* roster, CAT_item_proc action)
{
	item_grid_status = true;
	item_grid_title = title;
	item_grid_roster = roster;
	item_grid_action = action;

	CAT_ilist(&item_grid_pool, item_grid_pool_backing, CAT_ITEM_TABLE_CAPACITY);

	item_grid_text = "There's nothing here...";
}

void CAT_gui_item_grid_set_flags(int flags)
{
	item_grid_flags = flags;
}

void CAT_gui_item_grid_cell(int item_id)
{
	if(item_id < 0 || item_id >= item_table.length)
		return;
	CAT_ilist_push(&item_grid_pool, item_id);
}

static int item_grid_get_hovered_idx()
{
	int x = ITEM_GRID_MARGIN;
	int y = ITEM_GRID_HEADER_HEIGHT + ITEM_GRID_MARGIN + item_grid_delta_y;
	int idx = 0;

	while (idx < item_grid_pool.length)
	{
		for (int col = 0; col < ITEM_GRID_COLS; col++)
		{
			if(CAT_input_cursor_in_rect(0, 0, CAT_LCD_SCREEN_W, ITEM_GRID_HEADER_HEIGHT))
				return -1;
			if (CAT_input_cursor_in_rect(x, y, ITEM_GRID_CELL_SIZE, ITEM_GRID_CELL_SIZE))
				return idx;

			x += ITEM_GRID_CELL_SIZE + ITEM_GRID_MARGIN;
			idx += 1;
			if (idx >= item_grid_pool.length)
				break;
		}
		x = ITEM_GRID_MARGIN;
		y += ITEM_GRID_CELL_SIZE + ITEM_GRID_MARGIN;
	}

	return -1;
}

static int item_grid_get_min_scroll_y()
{
	return -ITEM_GRID_MARGIN;
}

static int item_grid_get_max_scroll_y()
{
	int rows = ceil(item_grid_pool.length / (float) ITEM_GRID_COLS);
	int pool_size = (ITEM_GRID_CELL_SIZE + ITEM_GRID_MARGIN) * rows + ITEM_GRID_MARGIN + ITEM_GRID_HEADER_HEIGHT - CAT_LCD_SCREEN_H;
	return pool_size > 0 ? pool_size : ITEM_GRID_MARGIN;
}

void CAT_gui_item_grid_io()
{
	if (CAT_input_touching())
	{
		// We always want to know this
		int hovered_idx = item_grid_get_hovered_idx();

		// If this is the click frame, register any clicked box
		if (CAT_input_touch_down())
		{
			item_grid_anchor_y = input.touch.y;
			item_grid_selector = hovered_idx;
			item_grid_last_marked = hovered_idx;
		}

		// Detect if this is a scroll action and quit early if so
		int scroll_y = input.touch.y;
		int scroll_dy = scroll_y - item_grid_anchor_y;
		if (abs(scroll_dy) > 4)
		{
			item_grid_anchor_y = scroll_y;
			item_grid_delta_y += scroll_dy;
			item_grid_delta_y = -clamp(-item_grid_delta_y, item_grid_get_min_scroll_y(), item_grid_get_max_scroll_y());

			item_grid_scrolling = true;
			item_grid_selector = -1;
			item_grid_select_timer = 0;
			return;
		}

		// If we have left the selected box,
		// cancel selection and inspection
		if (hovered_idx != item_grid_selector)
		{
			item_grid_selector = -1;
			item_grid_select_timer = 0;
		}
		// Otherwise continue to inspection logic
		else if (!item_grid_scrolling && item_grid_selector != -1)
		{
			if (input.touch_time >= 0.25f)
			{
				item_grid_select_timer += CAT_get_delta_time_s();
				if(item_grid_select_timer >= ITEM_GRID_SELECT_TIME)
				{
					if(item_grid_action != NULL)
						item_grid_action(item_grid_pool.data[item_grid_selector]);
					item_grid_select_timer = 0;
				}
			}
		}
	}
	else if (CAT_input_touch_up())
	{
		if(!item_grid_scrolling)
		{
			if(item_grid_selector != -1)
			{
				if(item_grid_roster != NULL)
				{
					int idx = CAT_ilist_find(item_grid_roster, item_grid_selector);
					if(idx < 0)
						CAT_ilist_push(item_grid_roster, item_grid_selector);
					else
						CAT_ilist_delete(item_grid_roster, idx);
				}
				else
				{
					item_grid_last_marked = item_grid_selector;
				}
			}
		}

		item_grid_scrolling = false;
		item_grid_selector = -1;
		item_grid_select_timer = 0;
	}

	if(!CAT_input_touching())
	{
		if (CAT_input_held(CAT_BUTTON_UP, 0))
			item_grid_delta_y += 32;
		if (CAT_input_held(CAT_BUTTON_DOWN, 0))
			item_grid_delta_y -= 32;
		item_grid_delta_y = -clamp(-item_grid_delta_y, item_grid_get_min_scroll_y(), item_grid_get_max_scroll_y());
	}

	if(CAT_input_pressed(CAT_BUTTON_A))
	{
		if(!CAT_gui_dialogue_is_open())
			CAT_gui_open_dialogue("Try touching and holding on the item you want to select.\n", 1);
	}
}

bool CAT_gui_item_grid_is_open()
{
	return item_grid_status;
}

void CAT_gui_item_grid()
{
	CAT_frameberry(ITEM_GRID_BG_COLOUR);

	int x = ITEM_GRID_MARGIN;
	int y = ITEM_GRID_MARGIN + ITEM_GRID_HEADER_HEIGHT + item_grid_delta_y;
	int idx = 0;

	while (idx < item_grid_pool.length)
	{
		for (int col = 0; col < ITEM_GRID_COLS; col++)
		{
			if(y >= -64 && y < CAT_LCD_SCREEN_H)
			{
				CAT_draw_sprite(&ui_item_frame_bg_sprite, 0, x, y);

				CAT_item* item = CAT_item_get(item_grid_pool.data[idx]);
				float aspect = item->sprite->height / (float) item->sprite->width;
				int major_axis = max(item->sprite->width, item->sprite->height);

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
				
				if(item_grid_roster != NULL && CAT_ilist_find(item_grid_roster, idx) >= 0)
				{
					CAT_draw_sprite(&ui_item_frame_fg_sprite, 0, x, y);
				}
				else if(idx == item_grid_last_marked)
				{
					CAT_draw_sprite(&ui_item_frame_fg_sprite, 0, x, y);
				}
			}

			idx += 1;
			x += 64 + 12;
			if (idx >= item_grid_pool.length)
				break;
		}

		x = ITEM_GRID_MARGIN;
		y += 64 + ITEM_GRID_MARGIN;
	}

	if(item_grid_pool.length == 0)
	{
		CAT_set_text_colour(CAT_WHITE);
		CAT_draw_text(ITEM_GRID_MARGIN, ITEM_GRID_MARGIN + ITEM_GRID_HEADER_HEIGHT + item_grid_delta_y, item_grid_text);
	}

	bool tabs = item_grid_flags & CAT_GUI_ITEM_GRID_FLAG_TABS;
	CAT_fillberry(0, 0, CAT_LCD_SCREEN_W, ITEM_GRID_HEADER_HEIGHT, ITEM_GRID_BG_COLOUR);
	CAT_set_text_colour(CAT_WHITE);
	CAT_set_text_scale(2);
	if(tabs)
		CAT_set_text_flags(CAT_TEXT_FLAG_CENTER);
	CAT_draw_text(tabs ? 120 : ITEM_GRID_MARGIN, ITEM_GRID_MARGIN, item_grid_title);
	CAT_lineberry(ITEM_GRID_MARGIN, ITEM_GRID_HEADER_HEIGHT-8, CAT_LCD_SCREEN_W-ITEM_GRID_MARGIN, ITEM_GRID_HEADER_HEIGHT-8, CAT_WHITE);
	if(tabs)
		CAT_draw_arrows(120, ITEM_GRID_MARGIN + 12, 8, CAT_LCD_SCREEN_W-ITEM_GRID_MARGIN*2-8*2, CAT_WHITE);

	float select_progress = item_grid_select_timer / ITEM_GRID_SELECT_TIME;
	if (item_grid_action != NULL && select_progress >= 0.05f)
		CAT_annulusberry(input.touch.x, input.touch.y, 24, 18, ITEM_GRID_ACTION_COLOUR, select_progress + 0.15f, 0);

	if (abs(-item_grid_delta_y - item_grid_get_max_scroll_y()) >= 64)
	{
		CAT_draw_sprite(&ui_down_arrow_sprite, -1, 240 - 32, 320 - 24);
	}

	if(CAT_is_last_render_cycle())
	{
		item_grid_status = false;
		item_grid_flags = CAT_GUI_ITEM_GRID_FLAG_NONE;
	}
}

void CAT_gui_item_grid_refresh()
{
	item_grid_delta_y = item_grid_get_min_scroll_y();
	item_grid_selector = -1;
	item_grid_select_timer = 0;
	item_grid_last_marked = -1;
}

void CAT_gui_item_grid_set_text(const char* text)
{
	item_grid_text = text;
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

void CAT_gui_dialogue_io()
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

void CAT_gui_io()
{
	if(CAT_gui_popup_is_open())
		CAT_gui_popup_io();
	else if(CAT_gui_keyboard_is_open())
		CAT_gui_keyboard_io();
	else if(CAT_gui_item_grid_is_open())
		CAT_gui_item_grid_io();
	else if(CAT_gui_menu_is_open())
		CAT_gui_menu_io();

	if(CAT_gui_dialogue_is_open())
		CAT_gui_dialogue_io();
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
	
	if(CAT_is_last_render_cycle())
		printf_cursor_y = 0;
}
