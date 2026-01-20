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
void CAT_gui_menu_disable_wrap();

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
// IT'S ABOUT TO GET WEIRD IN HERE

void CAT_GUI_new_frame();

void CAT_GUI_open_window(const char* text);
void CAT_GUI_close_current_window();
bool CAT_GUI_begin_window(const char* text, int x0, int y0, int x1, int y1);
void CAT_GUI_end_window();

void CAT_GUI_text(const char* text);
bool CAT_GUI_option(const char* text);
void CAT_GUI_image(const CAT_sprite* sprite, int frame_idx);

bool CAT_GUI_is_active();
void CAT_GUI_IO();
void CAT_GUI_draw();


//////////////////////////////////////////////////////////////////////////
// FINALIZATION

void CAT_gui_tick();
void CAT_gui_render();