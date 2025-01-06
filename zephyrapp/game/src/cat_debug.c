#include "cat_menu.h"
#include "cat_input.h"
#include "cat_version.h"
#include "cat_gui.h"
#include "cat_main.h"
#include "cat_pet.h"
#include "cat_room.h"

static enum
{
	SAVE,
	TIME,
	DECO,
	LAST
} page = SAVE;

void CAT_MS_debug(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			break;
		case CAT_MACHINE_SIGNAL_TICK:
			if(CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_back();

			if(CAT_input_pulse(CAT_BUTTON_LEFT))
			{
				if(page == 0)
					page = LAST-1;
				else
					page -= 1;
			}
			if(CAT_input_pulse(CAT_BUTTON_RIGHT))
			{
				page += 1;
				if(page >= LAST)
					page = 0;
			}	
			break;
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_debug()
{
	switch(page)
	{
		case SAVE:
			CAT_gui_title(true, NULL, &icon_exit_sprite, "SAVE");
			CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});
			CAT_gui_textf
			(
				"Game v%d.%d.%d.%d\nSave v%d.%d.%d.%d\n",
				CAT_VERSION_MAJOR, CAT_VERSION_MINOR,
				CAT_VERSION_PATCH, CAT_VERSION_PUSH,
				saved_version_major, saved_version_minor,
				saved_version_patch, saved_version_push
			);
		break;
		case TIME:
			CAT_gui_title(true, NULL, &icon_exit_sprite, "TIME");
			CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});
			CAT_gui_textf("Slept: %ds\n", logged_sleep);
			CAT_gui_textf("Life: %0.0fs/%0.0fs\n", CAT_timer_get(pet.life_timer_id), timetable.duration[pet.life_timer_id]);
			CAT_gui_textf("Stat: %0.0fs/%0.0fs\n", CAT_timer_get(pet.stat_timer_id), timetable.duration[pet.stat_timer_id]);
			CAT_gui_textf("Earn: %0.0fs/%0.0fs\n", CAT_timer_get(room.earn_timer_id), timetable.duration[room.earn_timer_id]);
			CAT_gui_textf("Pet: %0.0fs/%0.0fs\n", CAT_timer_get(pet.petting_timer_id), timetable.duration[pet.petting_timer_id]);
			CAT_gui_textf("Pets: %d/5\n", pet.times_pet, 5);
			CAT_gui_textf("Milks: %d/3\n", pet.times_milked, 3);
		break;
		case DECO:
			CAT_gui_title(true, NULL, &icon_exit_sprite, "DECO");
			CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});
			for(int y = 0; y < CAT_GRID_HEIGHT; y++)
			{
				for(int x = 0; x < CAT_GRID_WIDTH; x++)
				{
					int idx = y * CAT_GRID_WIDTH + x;
					int cell = space.cells[idx].occupied ? 1 : 0;
					CAT_gui_image(&icon_cell_sprite, cell);
				}
				CAT_gui_line_break();
			}
			CAT_gui_textf
			(
				"%d occupied, %d free\n%d total\n",
				CAT_GRID_SIZE - space.free_cell_count, space.free_cell_count,
				CAT_GRID_SIZE
			);
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
			CAT_gui_set_flag(CAT_GUI_WRAP_TEXT);
			CAT_gui_text("You shouldn't be here");
			break;
		}
	}	
}