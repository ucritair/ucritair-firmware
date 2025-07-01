#pragma once

#include "cat_math.h"
#include "cat_render.h"
#include "cat_structures.h"


//////////////////////////////////////////////////////////////////////////
// BASICS

typedef enum CAT_gui_flag
{
	CAT_GUI_FLAG_NONE = 0,
	CAT_GUI_FLAG_BORDERED = (1 << 0),
	CAT_GUI_FLAG_TIGHT = (1 << 1),
	CAT_GUI_FLAG_WRAPPED = (1 << 2),
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
void CAT_gui_title(bool tabs, const char* fmt, ...);


//////////////////////////////////////////////////////////////////////////
// KEYBOARD

void CAT_gui_open_keyboard(char* target);
bool CAT_gui_keyboard_is_open();


//////////////////////////////////////////////////////////////////////////
// POPUP

void CAT_gui_open_popup(const char* msg);
bool CAT_gui_popup_is_open();
bool CAT_gui_consume_popup();


//////////////////////////////////////////////////////////////////////////
// MENU

typedef enum
{
	CAT_GUI_MENU_TYPE_DEFAULT,
	CAT_GUI_MENU_TYPE_TOGGLE,
	CAT_GUI_MENU_TYPE_TICKER,
	CAT_GUI_MENU_TYPE_TEXT
} CAT_gui_menu_type;

typedef enum
{
	CAT_GUI_TOGGLE_STYLE_CHECKBOX,
	CAT_GUI_TOGGLE_STYLE_RADIO_BUTTON
} CAT_gui_toggle_style;

void CAT_gui_begin_menu_context();
void CAT_gui_clear_menu_context();
bool CAT_gui_begin_menu(const char* title);
bool CAT_gui_menu_is_open();
bool CAT_gui_menu_item(const char* title);
bool CAT_gui_menu_toggle(const char* title, bool toggle, CAT_gui_toggle_style style);
bool CAT_gui_menu_ticker(const char* title, int* ticker, int min, int max);
bool CAT_gui_menu_text(const char* fmt, ...);
void CAT_gui_end_menu();


//////////////////////////////////////////////////////////////////////////
// PRINTING

void CAT_gui_printf(uint16_t colour, const char* fmt, ...);


//////////////////////////////////////////////////////////////////////////
// ITEM GRID

typedef void (*CAT_item_proc)(int item_id);

typedef enum
{
	CAT_GUI_ITEM_GRID_FLAG_NONE = 0,
	CAT_GUI_ITEM_GRID_FLAG_TABS = (1 << 0)
} CAT_gui_item_grid_flag;

void CAT_gui_begin_item_grid_context();
void CAT_gui_begin_item_grid(const char* title, CAT_int_list* roster, CAT_item_proc action);
void CAT_gui_item_grid_set_flags(int flags);
void CAT_gui_item_grid_cell(int item_id);
bool CAT_gui_item_grid_is_open();
void CAT_gui_item_grid_refresh();
void CAT_gui_item_grid_set_text(const char* text);


//////////////////////////////////////////////////////////////////////////
// DIALOGUE BOX

void CAT_gui_open_dialogue(const char* text, int duration);
bool CAT_gui_dialogue_is_open();
void CAT_gui_dismiss_dialogue();


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
int CAT_break_list_lookup(int idx);
int CAT_break_list_count();
int CAT_break_list_get(int idx);


//////////////////////////////////////////////////////////////////////////
// TEXT

typedef enum
{
	CAT_TEXT_FLAG_NONE = 0,
	CAT_TEXT_FLAG_WRAP = (1 << 0),
	CAT_TEXT_FLAG_CENTER = (1 << 1), 
} CAT_text_flag;

void CAT_set_text_flags(int flags);
void CAT_set_text_colour(uint16_t colour);
void CAT_set_text_scale(uint8_t scale);
void CAT_set_text_mask(int x0, int y0, int x1, int y1);

int CAT_draw_text(int x, int y, const char* text);
int CAT_draw_textf(int x, int y, const char* fmt, ...);

