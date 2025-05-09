#pragma once

#include "cat_math.h"
#include "cat_render.h"

//////////////////////////////////////////////////////////////////////////
// CONSTANTS

#define CAT_GLYPH_WIDTH 8
#define CAT_GLYPH_HEIGHT 12
#define CAT_TEXT_INPUT_MAX 28


//////////////////////////////////////////////////////////////////////////
// BASICS

typedef enum CAT_gui_flag
{
	CAT_GUI_DEFAULT,
	CAT_GUI_PANEL_BORDER,
	CAT_GUI_TEXT_WRAP,
	CAT_GUI_PANEL_TIGHT,
	CAT_GUI_ITEM_LIST_COUNT,
	CAT_GUI_ITEM_LIST_PRICE,
	CAT_GUI_ITEM_LIST_COINS
} CAT_gui_flag;

void CAT_gui_set_flag(CAT_gui_flag flag);
bool CAT_gui_consume_flag(CAT_gui_flag flag);
CAT_gui_flag CAT_gui_clear_flags();

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

bool CAT_gui_begin_menu(const char* title);
bool CAT_gui_menu_is_open();
bool CAT_gui_menu_item(const char* title);
bool CAT_gui_menu_toggle(const char* title, bool toggle);
bool CAT_gui_menu_ticker(const char* title, int* ticker, int min, int max);
bool CAT_gui_menu_text(const char* fmt, ...);
void CAT_gui_end_menu();


//////////////////////////////////////////////////////////////////////////
// ITEM LIST

void CAT_gui_begin_item_list(const char* title);
bool CAT_gui_item_list_is_open();
bool CAT_gui_item_listing(int item_id, int count);
void CAT_gui_item_greyout();
void CAT_gui_item_highlight(float progress);


//////////////////////////////////////////////////////////////////////////
// PANEL-FREE GUI

void CAT_gui_printf(uint16_t colour, const char* fmt, ...);


//////////////////////////////////////////////////////////////////////////
// FINALIZATION

void CAT_gui_io();
void CAT_gui_render();