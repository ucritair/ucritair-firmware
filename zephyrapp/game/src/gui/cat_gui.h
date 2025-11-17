#pragma once

#include "cat_math.h"
#include "cat_render.h"
#include "cat_structures.h"


//////////////////////////////////////////////////////////////////////////
// KEYBOARD

void CAT_gui_open_keyboard(char* target, size_t max_size);
bool CAT_gui_keyboard_is_open();


//////////////////////////////////////////////////////////////////////////
// POPUP

typedef enum
{
	CAT_POPUP_STYLE_YES_NO,
	CAT_POPUP_STYLE_OK
} CAT_popup_style;

void CAT_gui_open_popup(const char* text, int style);
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

void CAT_gui_menu_override_exit(void (*exit_proc)());
void CAT_gui_menu_force_reset();

bool CAT_gui_begin_menu(const char* title);
void CAT_gui_end_menu();
bool CAT_gui_menu_is_open();

bool CAT_gui_menu_item(const char* title);
bool CAT_gui_menu_toggle(const char* title, bool toggle, CAT_gui_toggle_style style);
int CAT_gui_menu_ticker(const char* title, int value, int min, int max);
void CAT_gui_menu_text(const char* title);


//////////////////////////////////////////////////////////////////////////
// ITEM GRID

typedef void (*CAT_item_proc)(int item_id);

void CAT_gui_begin_item_grid_context(bool handle_exit);

void CAT_gui_begin_item_grid();
void CAT_gui_item_grid_add_tab(const char* title, CAT_item_proc focus_proc, CAT_item_proc confirm_proc);
int CAT_gui_item_grid_get_tab();

void CAT_gui_item_grid_cell(int item_id);
void CAT_gui_item_grid_highlight();
bool CAT_gui_item_grid_is_open();


//////////////////////////////////////////////////////////////////////////
// DIALOGUE BOX

void CAT_gui_open_dialogue(const char* text, int duration);
bool CAT_gui_dialogue_is_open();
void CAT_gui_dismiss_dialogue();


//////////////////////////////////////////////////////////////////////////
// FINALIZATION

void CAT_gui_tick();
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
	CAT_TEXT_FLAG_VERTICAL = (1 << 2)
} CAT_text_flag;

void CAT_set_text_flags(int flags);
void CAT_set_text_colour(uint16_t colour);
void CAT_set_text_scale(uint8_t scale);
void CAT_set_text_mask(int x0, int y0, int x1, int y1);

int CAT_draw_text(int x, int y, const char* text);
int CAT_draw_textf(int x, int y, const char* fmt, ...);
size_t CAT_get_drawn_strlen();

#define CAT_FLOAT_FMT "%d.%2.2u"
#define CAT_FMT_FLOAT(f) (int) (f), ((unsigned)(100 * ((f) - (int) (f))) % 100)
#define CAT_LINE_CAPACITY(l, r, w) ((CAT_LCD_SCREEN_W - (l + r))/w)