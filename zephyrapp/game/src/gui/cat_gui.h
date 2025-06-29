#pragma once

#include "cat_math.h"
#include "cat_render.h"

//////////////////////////////////////////////////////////////////////////
// CONSTANTS

#define CAT_TEXT_INPUT_MAX 28


//////////////////////////////////////////////////////////////////////////
// BASICS

typedef enum CAT_gui_flag
{
	CAT_GUI_FLAG_NONE = 0,
	CAT_GUI_FLAG_BORDERED = 1,
	CAT_GUI_FLAG_TIGHT = 2,
	CAT_GUI_FLAG_WRAPPED = 4,
	CAT_GUI_FLAG_INCLUDE_COUNT = 8,
	CAT_GUI_FLAG_INCLUDE_PRICE = 16,
	CAT_GUI_FLAG_SHOW_COINS = 32,
} CAT_gui_flag;

void CAT_gui_set_flag(int flag);
bool CAT_gui_consume_flag(int flag);
int CAT_gui_clear_flags();

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

void CAT_gui_panel(CAT_ivec2 start, CAT_ivec2 shape);
void CAT_gui_line_break();

void CAT_gui_text(const char* text);
void CAT_gui_image(const CAT_sprite* sprite, int frame_idx);
void CAT_gui_div(const char* text);
void CAT_gui_textf(const char* fmt, ...);
void CAT_gui_title(bool tabs, const CAT_sprite* a_action, const CAT_sprite* b_action, const char* fmt, ...);


//////////////////////////////////////////////////////////////////////////
// KEYBOARD

void CAT_gui_open_keyboard(char* target);
bool CAT_gui_keyboard_is_open();


//////////////////////////////////////////////////////////////////////////
// POPUP

void CAT_gui_open_popup(const char* msg, bool* result);
bool CAT_gui_popup_is_open();


//////////////////////////////////////////////////////////////////////////
// MENU

typedef enum
{
	CAT_GUI_MENU_TYPE_DEFAULT,
	CAT_GUI_MENU_TYPE_TOGGLE,
	CAT_GUI_MENU_TYPE_TICKER,
	CAT_GUI_MENU_TYPE_TEXT
} CAT_gui_menu_type;

void CAT_gui_begin_menu_context();
bool CAT_gui_begin_menu(const char* title);
bool CAT_gui_menu_is_open();
bool CAT_gui_menu_item(const char* title);
bool CAT_gui_menu_toggle(const char* title, bool toggle);
bool CAT_gui_menu_ticker(const char* title, int* ticker, int min, int max);
bool CAT_gui_menu_text(const char* fmt, ...);
void CAT_gui_end_menu();
void CAT_gui_end_menu_context();


//////////////////////////////////////////////////////////////////////////
// ITEM LIST

void CAT_gui_begin_item_list_context();
void CAT_gui_begin_item_list(const char* title);
bool CAT_gui_item_list_is_open();
bool CAT_gui_item_listing(int item_id);
void CAT_gui_item_greyout();
void CAT_gui_item_highlight(float progress);
void CAT_gui_end_item_list_context();


//////////////////////////////////////////////////////////////////////////
// PRINTING

void CAT_gui_printf(uint16_t colour, const char* fmt, ...);


//////////////////////////////////////////////////////////////////////////
// FINALIZATION

void CAT_gui_io();
void CAT_gui_render();


//////////////////////////////////////////////////////////////////////////
// TEXT CONSTANTS

#define CAT_TEXT_LINE_HEIGHT (CAT_GLYPH_HEIGHT + 2)
#define CAT_TEXT_MAX_LINES (CAT_LCD_SCREEN_H / CAT_TEXT_LINE_HEIGHT)


//////////////////////////////////////////////////////////////////////////
// TEXT WRAPPING

void CAT_break_list_init(const char* txt, int line_width, int scale);
bool CAT_break_list_lookup(int idx);
int CAT_break_list_count();
int CAT_break_list_get(int idx);


//////////////////////////////////////////////////////////////////////////
// TEXT

typedef enum
{
	CAT_TEXT_FLAG_NONE = 0,
	CAT_TEXT_FLAG_WRAP = 1
} CAT_text_flag;

void CAT_set_text_flags(int flags);
void CAT_set_text_colour(uint16_t colour);
void CAT_set_text_scale(uint8_t scale);
void CAT_set_text_mask(int x0, int y0, int x1, int y1);

int CAT_draw_text(int x, int y, const char* text);
int CAT_draw_textf(int x, int y, const char* fmt, ...);

