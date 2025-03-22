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
#include "cat_bag.h"

//////////////////////////////////////////////////////////////////////////
// BASICS

CAT_gui gui =
{
	.flags = CAT_GUI_DEFAULT,

	.start = (CAT_ivec2) {0, 0},
	.shape = (CAT_ivec2) {0, 0},
	.cursor = (CAT_ivec2) {0, 0},

	.margin = 8,
	.pad = 4,
	.channel_height = 0
};

void CAT_gui_set_flag(CAT_gui_flag flag)
{
	gui.flags |= (1 << flag);
}

bool flag_is_set(CAT_gui_flag field, CAT_gui_flag flag)
{
	return (field & (1 << flag)) > 0;
}

bool CAT_gui_consume_flag(CAT_gui_flag flag)
{
	bool value =  flag_is_set(gui.flags, flag);
	gui.flags &= ~(1 << flag);
	return value;
}

CAT_gui_flag CAT_gui_clear_flags()
{
	CAT_gui_flag value = gui.flags;
	gui.flags = 0;
	return value;
}

void CAT_gui_panel(CAT_ivec2 start, CAT_ivec2 shape)
{
	draw_mode = CAT_DRAW_MODE_DEFAULT;
	CAT_fillberry(start.x * CAT_TILE_SIZE, start.y * CAT_TILE_SIZE, shape.x * CAT_TILE_SIZE, shape.y * CAT_TILE_SIZE, 0xFFFF);
	if(CAT_gui_consume_flag(CAT_GUI_PANEL_BORDER))
		CAT_strokeberry(start.x * CAT_TILE_SIZE, start.y * CAT_TILE_SIZE, shape.x * CAT_TILE_SIZE, shape.y * CAT_TILE_SIZE, 0x0000);

	gui.start = start;
	gui.shape = shape;
	gui.cursor = CAT_ivec2_mul(start, CAT_TILE_SIZE);
	gui.cursor.y += gui.margin;
	gui.cursor.x += gui.margin;
	if(CAT_gui_consume_flag(CAT_GUI_PANEL_TIGHT))
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
	draw_mode = CAT_DRAW_MODE_CENTER_Y;

	bool wrap = CAT_gui_consume_flag(CAT_GUI_TEXT_WRAP);
	int x_lim = (gui.start.x * CAT_TILE_SIZE) + (gui.shape.x) * CAT_TILE_SIZE - CAT_GLYPH_WIDTH - gui.margin;
	const char* c = text;

	while(*c != '\0')
	{
		if(wrap && gui.cursor.x >= x_lim && !isspace(*(c+1)))
		{
			if(!isspace(*c) && !isspace(*(c-1)))
				CAT_draw_sprite(&glyph_sprite, '-', gui.cursor.x, gui.cursor.y);
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
		if(*c == '\t')
		{
			gui.cursor.x += CAT_GLYPH_WIDTH * 4;
			c++;
			continue;
		}

		gui_open_channel(CAT_GLYPH_HEIGHT);
		CAT_draw_sprite(&glyph_sprite, *c, gui.cursor.x, gui.cursor.y);
		gui.cursor.x += CAT_GLYPH_WIDTH;
		c++;
	}
}

void CAT_gui_image(const CAT_sprite* sprite, int frame_idx)
{
	draw_mode = CAT_DRAW_MODE_CENTER_Y;

	gui_open_channel(sprite->height);

	gui.cursor.x += gui.pad / 2;
	CAT_draw_sprite(sprite, frame_idx, gui.cursor.x, gui.cursor.y);
	gui.cursor.x += sprite->width;
	gui.cursor.x += gui.pad / 2;
}

void CAT_gui_div(const char* text)
{
	draw_mode = CAT_DRAW_MODE_CENTER_Y;
	
	CAT_gui_line_break();
	gui_open_channel(CAT_TILE_SIZE);
	if(strlen(text) == 0)
	{
		CAT_rowberry(0, gui.cursor.y, CAT_LCD_SCREEN_W, 0x0000);
	}
	else
	{
		CAT_gui_text(text);
		int start = gui.cursor.x + gui.pad;
		CAT_rowberry(start, gui.cursor.y, CAT_LCD_SCREEN_W-start-gui.margin, 0x0000);
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

void CAT_gui_title(bool tabs, const CAT_sprite* a_action, const CAT_sprite* b_action, const char* fmt, ...)
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});
	CAT_rowberry(0, 31, CAT_LCD_SCREEN_W, 0x0000);
	
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

	if(a_action != NULL)
	{
		CAT_gui_image(&icon_a_sprite, 1);
		CAT_gui_image(a_action, 0);
	}
	if(b_action != NULL)
	{
		CAT_gui_image(&icon_b_sprite, 1);
		CAT_gui_image(b_action, 0);
	}
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
	strcpy(keyboard.buffer, target);
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
	if(CAT_input_pressed(CAT_BUTTON_UP))
		keyboard.row_idx -= 1;
	if(CAT_input_pressed(CAT_BUTTON_DOWN))
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
	if(CAT_input_pressed(CAT_BUTTON_RIGHT))
		keyboard.glyph_idx += 1;
	if(CAT_input_pressed(CAT_BUTTON_LEFT))
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
			if(keyboard.cursor < CAT_TEXT_INPUT_MAX)
			{
				keyboard.buffer[keyboard.cursor] = glyph;
				keyboard.cursor += 1;
			}
		}
		keyboard.buffer[keyboard.cursor] = '\0';
	}

	keyboard.cursor_timer += CAT_get_delta_time();
	if(keyboard.cursor_timer >= 0.5f)
	{
		keyboard.cursor_timer = 0.0f;
		keyboard.show_cursor = !keyboard.show_cursor;
	}
}

void CAT_gui_keyboard()
{	
	CAT_gui_panel((CAT_ivec2){0, 10}, (CAT_ivec2){15, 10});
	CAT_rowberry(0, 160, CAT_LCD_SCREEN_W, 0x0000);
	CAT_gui_text(keyboard.buffer);
	if(keyboard.show_cursor)
		CAT_gui_text("|");
	gui.cursor.y -= 4;
	CAT_gui_div("");

	draw_mode = CAT_DRAW_MODE_DEFAULT;
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
	bool* result;
	uint8_t selector;
	bool open;
} popup =
{
	NULL,
	NULL,
	0,
	false
};

void CAT_gui_open_popup(const char* msg, bool* result)
{	
	popup.msg = msg;
	popup.result = result;
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
		*(popup.result) = popup.selector;
		popup.open = false;
	}
	if(CAT_input_pressed(CAT_BUTTON_START) || CAT_input_pressed(CAT_BUTTON_B))
	{
		*(popup.result) = false;
		popup.open = false;
	}
}

void CAT_gui_popup()
{
	CAT_gui_panel(CAT_iv2(2, 6), CAT_iv2(11, 8));
	CAT_strokeberry(2 * 16, 6 * 16, 11 * 16, 8 * 16, 0x0000);
	CAT_gui_set_flag(CAT_GUI_TEXT_WRAP);
	CAT_gui_text(popup.msg);
	CAT_gui_line_break();
	CAT_gui_text(popup.selector ? "[YES]  NO " : " YES  [NO]");
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
	bool selected;
	CAT_gui_flag flags;

	int parent;
	uint16_t children[32];
	uint8_t child_count;
	uint8_t selector;
} menu_node;

static menu_node menu_table[512];
#define MENU_TABLE_SIZE (sizeof(menu_table) / sizeof(menu_table[0]))
static uint16_t menu_stack[128];
#define MENU_STACK_SIZE (sizeof(menu_stack) / sizeof(menu_stack[0]))
static uint8_t menu_stack_length = 0;
static int menu_root = -1;
static bool menu_open = false;

uint16_t register_menu_node(const char* title)
{
	uint16_t idx = hash(title) % MENU_TABLE_SIZE;
	while (menu_table[idx].live)
		idx++;

	menu_table[idx] = (menu_node)
	{
		.live = true,
		.title = title,
		.flags = CAT_GUI_DEFAULT,
		.selected = false,
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
	menu_stack[menu_stack_length] = table_idx;
	menu_stack_length += 1;
}

uint16_t pop_menu_node()
{
	menu_stack_length -= 1;
	return menu_stack[menu_stack_length];
}

uint8_t menu_add_child(uint16_t table_idx)
{
	menu_node* head = &menu_table[menu_stack[menu_stack_length-1]];

	uint8_t child_idx = head->child_count;
	head->children[child_idx] = table_idx;
	head->child_count += 1;
	menu_table[table_idx].parent = menu_stack[menu_stack_length-1];
	return child_idx;
}

int menu_find_child(uint16_t table_idx)
{
	menu_node* head = &menu_table[menu_stack[menu_stack_length-1]];

	for(int i = 0; i < head->child_count; i++)
	{
		if(head->children[i] == table_idx)
			return i;
	}
	return -1;
}

menu_node* menu_find_head()
{
	menu_node* ptr =&menu_table[menu_root];
	for(int i = 0; i < ptr->child_count; i++)
	{
		menu_node* child = &menu_table[ptr->children[i]];
		if(child->selected && child->child_count > 0)
		{
			ptr = child;
			i = -1;
			continue;
		}
	}
	return ptr;
}

bool CAT_gui_begin_menu(const char* title)
{
	menu_open = true;

	int idx = find_menu_node(title);
	if(idx == -1)
		idx = register_menu_node(title);
	menu_table[idx].child_count = 0;

	if (!CAT_gui_menu_is_open())
	{
		push_menu_node(idx);
		menu_table[idx].selected = true;
		menu_root = idx;
	}
	else
	{
		menu_add_child(idx);
		if(menu_table[idx].selected)
			push_menu_node(idx);
	}

	return menu_table[idx].selected;
}

bool CAT_gui_menu_is_open()
{
	return menu_open && menu_root != -1 && menu_table[menu_root].selected;
}

bool CAT_gui_menu_item(const char* title)
{
	int idx = find_menu_node(title);
	if(idx == -1)
		idx = register_menu_node(title);
	menu_add_child(idx);

	bool selected = menu_table[idx].selected;
	menu_table[idx].selected = false;
	menu_table[idx].flags = CAT_gui_clear_flags();
	return selected;
}

void CAT_gui_end_menu()
{
	pop_menu_node();
}

void CAT_gui_menu_io()
{
	menu_node* head = menu_find_head();

	if(CAT_input_pressed(CAT_BUTTON_UP))
		head->selector -= 1;
	if(CAT_input_pressed(CAT_BUTTON_DOWN))
		head->selector += 1;
	head->selector = clamp(head->selector, 0, head->child_count-1);

	menu_node* hovered = &menu_table[head->children[head->selector]];
	if(CAT_input_pressed(CAT_BUTTON_A))
		hovered->selected = true;
	if(CAT_input_pressed(CAT_BUTTON_B))
	{
		if(hovered->parent == menu_root)
			CAT_machine_back();
		else
			menu_table[hovered->parent].selected = false;
	}	
}

void CAT_gui_menu()
{
	menu_node* head = menu_find_head();

	CAT_gui_title
	(
		false,
		&icon_enter_sprite, &icon_exit_sprite,
		head->title
	);

	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

	for(int i = 0; i < head->child_count; i++)
	{
		menu_node* child = &menu_table[head->children[i]];

		CAT_gui_textf("\1 %s ", child->title);
		if(flag_is_set(child->flags, CAT_GUI_MENU_HIGHLIGHTABLE))
		{
			CAT_gui_image(&icon_equip_sprite, flag_is_set(child->flags, CAT_GUI_MENU_HIGHLIGHTED));
			CAT_gui_text(" ");
		}
		if(i == head->selector)
			CAT_gui_image(&icon_pointer_sprite, 0);	
		CAT_gui_line_break();
	}

	if(CAT_is_last_render_cycle())
	{
		menu_stack_length = 0;
		menu_root = -1;
		menu_open = false;
	}
}


//////////////////////////////////////////////////////////////////////////
// ITEM LIST

static bool item_list_open = false;
static const char* item_list_title = NULL;

static CAT_item_list item_list;
static bool item_list_selection_mask[CAT_ITEM_LIST_MAX_LENGTH];
static bool item_list_greyout_mask[CAT_ITEM_LIST_MAX_LENGTH];
static float item_list_highlight_mask[CAT_ITEM_LIST_MAX_LENGTH];

static int item_list_selector = 0;
static int item_display_base = 0;
static int item_display_count = 9;

static bool show_price = false;
static bool show_count = false;
static bool show_coins = false;

void CAT_gui_begin_item_list(const char* title)
{
	item_list_open = true;
	item_list_title = title;
	CAT_item_list_init(&item_list);

	for(int i = 0; i < CAT_ITEM_LIST_MAX_LENGTH; i++)
	{
		item_list_selection_mask[i] = false;
		item_list_greyout_mask[i] = false;
		item_list_highlight_mask[i] = 0;
	}

	show_price = CAT_gui_consume_flag(CAT_GUI_ITEM_LIST_PRICE);
	show_count = CAT_gui_consume_flag(CAT_GUI_ITEM_LIST_COUNT);
	show_coins = CAT_gui_consume_flag(CAT_GUI_ITEM_LIST_COINS);
	item_display_count = show_coins ? 8 : 9;
}

bool CAT_gui_item_list_is_open()
{
	return item_list_open;
}

bool CAT_gui_item_listing(int item_id, int count)
{
	CAT_item_list_add(&item_list, item_id, count);
	bool pressed = CAT_input_pressed(CAT_BUTTON_A) || CAT_input_held(CAT_BUTTON_A, 0);
	bool hovered = item_list_selector == item_list.length-1;
	return hovered && pressed;
}

void CAT_gui_item_greyout()
{
	item_list_greyout_mask[item_list.length-1] = true;
}

void CAT_gui_item_highlight(float progress)
{
	item_list_highlight_mask[item_list.length-1] = progress;
}

void CAT_gui_item_list_io()
{
	if(item_list.length == 0)
		return;

	if(CAT_input_pulse(CAT_BUTTON_UP))
		item_list_selector -= 1;
	if(CAT_input_pulse(CAT_BUTTON_DOWN))
		item_list_selector += 1;
	item_list_selector = (item_list_selector + item_list.length) % item_list.length;

	int overshoot = item_list_selector - item_display_base;
	if(overshoot < 0)
		item_display_base += overshoot;
	else if(overshoot >= item_display_count)
		item_display_base += (overshoot - (item_display_count-1));
}

char item_label[36];
int item_label_length = 0;
void ilstart()
{
	item_label_length = 0;
}
void ilprintf(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int added = vsnprintf(item_label + item_label_length, sizeof(item_label), fmt, args);
	item_label_length += added;
	va_end(args);
}
void ilend()
{
	item_label[item_label_length] = '\0';
}

void CAT_gui_item_list()
{
	CAT_gui_title
	(
		false,
		&icon_enter_sprite, &icon_exit_sprite,
		item_list_title
	); 

	if(show_coins)
	{
		CAT_gui_set_flag(CAT_GUI_PANEL_TIGHT);
		CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 2});
		CAT_gui_image(&icon_coin_sprite, 0);
		CAT_rowberry(0, (2*2)*16-1, CAT_LCD_SCREEN_W, 0x0000);
		CAT_gui_textf(" %d", coins);
		CAT_gui_panel((CAT_ivec2) {0, 4}, (CAT_ivec2) {15, 16}); 
	}
	else
	{
		CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});
	}
	int tile_row_offset = show_coins ? 4 : 2;

	if(item_list.length == 0)
	{
		CAT_gui_text
		(
			"You do not have any\n"
			"appropriate items."
		);
	}
	for(int display_idx = 0; display_idx < item_display_count; display_idx++)
	{
		int list_idx = item_display_base + display_idx;
		if(list_idx >= item_list.length)
			break;

		int item_id = item_list.item_ids[list_idx];
		CAT_item* item = CAT_item_get(item_id);
		
		CAT_gui_set_flag(CAT_GUI_PANEL_TIGHT);
		CAT_gui_panel((CAT_ivec2) {0, tile_row_offset+display_idx*2}, (CAT_ivec2) {15, 2});
		CAT_rowberry(0, (tile_row_offset+display_idx*2+2)*16-1, CAT_LCD_SCREEN_W, 0x0000);
		CAT_gui_image(item->icon, 0);
		
		ilstart();
		ilprintf(" %s ", item->name);
		if(show_price)
			ilprintf("$%d ", item->price);
		if(show_count)
			ilprintf("*%d ", item_list.counts[list_idx]);
		ilend();
		CAT_gui_text(item_label);

		if(list_idx == item_list_selector)
			CAT_gui_image(&icon_pointer_sprite, 0);

		if(item_list_greyout_mask[list_idx])
			CAT_greyberry(0, CAT_LCD_SCREEN_W, (tile_row_offset+display_idx*2)*16, 32);
		CAT_greenberry(0, CAT_LCD_SCREEN_H, (tile_row_offset+display_idx*2)*16, 32, item_list_highlight_mask[list_idx]);
	}

	if(CAT_is_last_render_cycle())
		item_list_open = false;	
}


//////////////////////////////////////////////////////////////////////////
// FINALIZATION

void CAT_gui_io()
{
	if(CAT_gui_keyboard_is_open())
		CAT_gui_keyboard_io();
	else if(CAT_gui_popup_is_open())
		CAT_gui_popup_io();
	else if(CAT_gui_item_list_is_open())
		CAT_gui_item_list_io();
	else if(CAT_gui_menu_is_open())
		CAT_gui_menu_io();
}

void CAT_gui_render()
{
	if(CAT_gui_menu_is_open())
		CAT_gui_menu();
	if(CAT_gui_item_list_is_open())
		CAT_gui_item_list();
	if(CAT_gui_popup_is_open())
		CAT_gui_popup();
	if(CAT_gui_keyboard_is_open())
		CAT_gui_keyboard();
}
