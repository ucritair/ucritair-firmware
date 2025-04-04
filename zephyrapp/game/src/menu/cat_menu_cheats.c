// DEPRECATED
/*
	This code uses the legacy menu "system".
	The cheats implemented here should be rewritten
	in the cheats section of the main menu using the new menu system.
	Thereafter, this file and all references to it should be removed.
*/

#include "cat_menu.h"
#include "cat_machine.h"
#include "cat_input.h"
#include "cat_gui.h"
#include "cat_main.h"
#include "cat_pet.h"
#include "cat_room.h"
#include "cat_bag.h"
#include "config.h"
#include "sprite_assets.h"

static enum {
    PAGE_1,
    PAGE_2,
    LAST
} page = PAGE_1;

static int selector = 0;

typedef void (*CheatCallback)(void);

typedef struct {
    const char *title;
    CheatCallback callback;
} CheatEntry;

// ==== PAGE 1 CHEATS ====
static void cheat_add_coins() { coins += 1000; }
static void cheat_base_stats() { pet.vigour = 9; pet.focus = 9; pet.spirit = 9; }
static void cheat_max_stats() { pet.vigour = 12; pet.focus = 12; pet.spirit = 12; }
static void cheat_crit_stats() { pet.vigour = 3; pet.focus = 3; pet.spirit = 3; }
static void cheat_all_items() {
    for (int i = 0; i < item_table.length; i++) {
        CAT_item_list_add(&bag, i, 1);
    }
}
static void cheat_turnkey() { needs_load = true; override_load = true; }

static CheatEntry cheats_page1[] = {
    { "+ 1000 COINS", cheat_add_coins },
    { "BASE STATS", cheat_base_stats },
    { "MAX STATS", cheat_max_stats },
    { "CRIT STATS", cheat_crit_stats },
    { "+ EVERY ITEM", cheat_all_items },
    { "TURNKEY APARTMENT", cheat_turnkey },
};

// ==== PAGE 2 CHEATS ====
static void cheat_add_level() { pet.level += 1; }
static void cheat_add_100_xp() { pet.xp += 100; }
static void cheat_add_1_xp() { pet.xp += 1; }
static void cheat_add_day() { pet.lifetime += 1; }
static void cheat_add_mask() { CAT_item_list_add(&bag, mask_item, 1); }
static void cheat_remove_mask() { CAT_item_list_remove(&bag, mask_item, 1); }

static CheatEntry cheats_page2[] = {
    { "ADD LEVEL", cheat_add_level },
    { "+ 100 XP", cheat_add_100_xp },
    { "+ 1 XP", cheat_add_1_xp },
    { "+ 1 DAY", cheat_add_day },
    { "ADD MASK", cheat_add_mask },
    { "REMOVE MASK", cheat_remove_mask },
};

void CAT_MS_cheats(CAT_machine_signal signal)
{
    switch (signal)
    {
        case CAT_MACHINE_SIGNAL_ENTER:
            CAT_set_render_callback(CAT_render_cheats);
            break;

        case CAT_MACHINE_SIGNAL_TICK:
        {
            CheatEntry* entries;
            int num_entries;
            switch (page)
            {
                case PAGE_1:
                    entries = cheats_page1;
                    num_entries = sizeof(cheats_page1) / sizeof(CheatEntry);
                    break;

                case PAGE_2:
                    entries = cheats_page2;
                    num_entries = sizeof(cheats_page2) / sizeof(CheatEntry);
                    break;

                    default:
                        return;
            }


			if (CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
			    CAT_machine_back();
							
            if (CAT_input_pulse(CAT_BUTTON_UP))
                selector = (selector - 1 + num_entries) % num_entries;

            if (CAT_input_pulse(CAT_BUTTON_DOWN))
                selector = (selector + 1) % num_entries;

            if (CAT_input_pulse(CAT_BUTTON_LEFT))
            {
                page = (page == 0) ? LAST - 1 : page - 1;
                selector = 0;
            }

            if (CAT_input_pulse(CAT_BUTTON_RIGHT))
            {
                page = (page + 1) % LAST;
                selector = 0;
            }

            if (CAT_input_pressed(CAT_BUTTON_A))
            {
                entries[selector].callback();
            }
            break;
        }
        case CAT_MACHINE_SIGNAL_EXIT:
		    break;
    }
}

void CAT_render_cheats(void)
{
    switch (page)
    {
        case PAGE_1:
            CAT_gui_title(true, &icon_enter_sprite, &icon_exit_sprite, "CHEATS: PAGE 1");
            CAT_gui_panel((CAT_ivec2){0, 2}, (CAT_ivec2){15, 18});
            for (int i = 0; i < sizeof(cheats_page1) / sizeof(CheatEntry); i++)
            {
                CAT_gui_textf("\t\1 %s", cheats_page1[i].title);
                if (i == selector)
                    CAT_gui_image(&icon_pointer_sprite, 0);
                CAT_gui_line_break();
            }
            break;

        case PAGE_2:
            CAT_gui_title(true, &icon_enter_sprite, &icon_exit_sprite, "CHEATS: PAGE 2");
            CAT_gui_panel((CAT_ivec2){0, 2}, (CAT_ivec2){15, 18});
            for (int i = 0; i < sizeof(cheats_page2) / sizeof(CheatEntry); i++)
            {
                CAT_gui_textf("\t\1 %s", cheats_page2[i].title);
                if (i == selector)
                    CAT_gui_image(&icon_pointer_sprite, 0);
                CAT_gui_line_break();
            }
            break;

        default:
        {
            CAT_gui_title
            (
                true,
                NULL, &icon_exit_sprite,
                "LAST"
            );
            CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});
            CAT_gui_set_flag(CAT_GUI_TEXT_WRAP);
            CAT_gui_text("You shouldn't be here");
            break;
        }
    }
}
