#include "cat_menu.h"

#include "cat_room.h"
#include "cat_gui.h"
#include "cat_input.h"
#include "cat_machine.h"
#include "cat_sprite.h"
#include "cat_version.h"
#include "cat_manual.h"
#include "cat_vending.h"
#include "cat_stats.h"
#include "cat_arcade.h"
#include "cat_bag.h"
#include "cat_pet.h"
#include "cat_main.h"
#include <stddef.h>
#include <string.h>
#include <math.h>

#ifdef CAT_EMBEDDED
#include "menu_system.h"
#include "menu_aqi.h"
#endif

struct entry
{
	const char* title;
	CAT_machine_state state;
} entries[] =
{
	{"INSIGHTS", CAT_MS_stats},
	{"BAG", CAT_MS_bag},
	{"VENDING MACHINE", CAT_MS_vending},
	{"ARCADE CABINET", CAT_MS_arcade},
#ifdef CAT_EMBEDDED
	{"AIR QUALITY", CAT_MS_aqi},
	{"SYSTEM MENU", CAT_MS_system_menu},
#endif
	{"MAGIC", CAT_MS_magic},
	{"MANUAL", CAT_MS_manual},
	{"BACK", CAT_MS_room},
#ifdef CAT_DEBUG
	{"DEBUG", CAT_MS_debug},
	{"CHEATS", CAT_MS_cheats},
#endif
};
#define NUM_MENU_ITEMS (sizeof(entries)/sizeof(entries[0]))
static int selector = 0;

const char* flavour_text[] =
{
	"Reticulating splines..."
};
#define FLAVOUR_TEXT_COUNT (sizeof(flavour_text)/sizeof(flavour_text[0]))
int flavour_idx = 0;

void CAT_MS_menu(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			flavour_idx = CAT_rand_int(0, FLAVOUR_TEXT_COUNT-1);
			break;
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_back();
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_room);

			if(CAT_input_pulse(CAT_BUTTON_UP))
				selector -= 1;
			if(CAT_input_pulse(CAT_BUTTON_DOWN))
				selector += 1;
			selector = clamp(selector, 0, NUM_MENU_ITEMS-1);

			if(CAT_input_pressed(CAT_BUTTON_A))
				CAT_machine_transition(entries[selector].state);
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_menu()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
	CAT_gui_text("MENU ");
	CAT_gui_image(icon_a_sprite, 1);
	CAT_gui_image(icon_enter_sprite, 0);
	CAT_gui_image(icon_b_sprite, 1);
	CAT_gui_image(icon_exit_sprite, 0);

	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

	for (int i = 0; i < NUM_MENU_ITEMS; i++)
	{
		CAT_gui_textf("& %s ", entries[i].title);

		if(i == selector)
			CAT_gui_image(icon_pointer_sprite, 0);

		CAT_gui_line_break();
	}

	CAT_gui_line_break();
	CAT_gui_set_flag(CAT_GUI_WRAP_TEXT);
	CAT_gui_text(flavour_text[flavour_idx]);
}

void CAT_MS_debug(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			break;
		case CAT_MACHINE_SIGNAL_TICK:
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_back();
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_room);
			break;
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_debug()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 20});

	CAT_gui_textf
	(
		"Game v%d.%d.%d.%d\nSave v%d.%d.%d.%d\n",
		CAT_VERSION_MAJOR, CAT_VERSION_MINOR,
		CAT_VERSION_PATCH, CAT_VERSION_PUSH,
		saved_version_major, saved_version_minor,
		saved_version_patch, saved_version_push
	);
	CAT_gui_line_break();

	CAT_gui_textf("Slept: %ds\n", logged_sleep);
	CAT_gui_textf("Life: %0.0fs/%0.0fs\n", CAT_timer_get(pet.life_timer_id), timetable.duration[pet.life_timer_id]);
	CAT_gui_textf("Stat: %0.0fs/%0.0fs\n", CAT_timer_get(pet.stat_timer_id), timetable.duration[pet.stat_timer_id]);
	CAT_gui_textf("Earn: %0.0fs/%0.0fs\n", CAT_timer_get(room.earn_timer_id), timetable.duration[room.earn_timer_id]);
	CAT_gui_line_break();

	for(int y = 0; y < CAT_GRID_HEIGHT; y++)
	{
		for(int x = 0; x < CAT_GRID_WIDTH; x++)
		{
			int idx = y * CAT_GRID_WIDTH + x;
			int cell = space.cells[idx].occupied ? 1 : 0;
			CAT_gui_image(icon_cell_sprite, cell);
		}
		CAT_gui_line_break();
	}
	CAT_gui_textf
	(
		"%d occupied, %d free\n%d total\n",
		CAT_GRID_SIZE - space.free_cell_count, space.free_cell_count,
		CAT_GRID_SIZE
	);
}

void CAT_MS_litany(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			break;
		case CAT_MACHINE_SIGNAL_TICK:
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_back();
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_room);
			break;
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void cheat_proc_1k_coins()
{
	coins += 1000;
}

void cheat_proc_base_stats()
{
	pet.vigour = 9;
	pet.focus = 9;
	pet.spirit = 9;
	CAT_pet_reanimate();
}

void cheat_proc_crit_stats()
{
	pet.vigour = 3;
	pet.focus = 3;
	pet.spirit = 3;
	CAT_pet_reanimate();
}

void cheat_proc_all_items()
{
	for(int item_id = 0; item_id < item_table.length; item_id++)
	{
		CAT_item_list_add(&bag, item_id, 1);
	}
}

struct
{
	const char* name;
	void (*proc)();
} cheat_entries[] =
{
	{"1000 COINS", cheat_proc_1k_coins},
	{"BASE STATS", cheat_proc_base_stats},
	{"CRITICAL STATS", cheat_proc_crit_stats},
	{"ALL ITEMS", cheat_proc_all_items},
};
int num_cheat_entries = sizeof(cheat_entries) / sizeof(cheat_entries[0]);
static int cheat_selector = 0;

void CAT_MS_cheats(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			break;
		case CAT_MACHINE_SIGNAL_TICK:
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_back();
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_room);

			if(CAT_input_pulse(CAT_BUTTON_UP))
				cheat_selector -= 1;
			if(CAT_input_pulse(CAT_BUTTON_DOWN))
				cheat_selector += 1;
			cheat_selector = clamp(cheat_selector, 0, num_cheat_entries-1);

			if(CAT_input_pressed(CAT_BUTTON_A))
			{
				cheat_entries[cheat_selector].proc();
			}
			break;
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_cheats()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 20});
	for(int i = 0; i < num_cheat_entries; i++)
	{
		CAT_gui_textf("& %s ", cheat_entries[i].name);
		if(i == cheat_selector)
			CAT_gui_image(icon_pointer_sprite, 0);
		CAT_gui_line_break();
	}
}


//////////////////////////////////////////////////////////////////////////
// WIP LEAVE ME ALONE

void CAT_print_vec4(CAT_vec4 vec)
{
	CAT_printf("%f %f %f %f\n", vec.x, vec.y, vec.z, vec.w);
}

void CAT_print_mat4(CAT_mat4 mat)
{
	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			CAT_printf("%f ", mat.data[i * 4 + j]);
		}	
		CAT_printf("\n");
	}
}

#include "../meshes/mesh_assets.h"

CAT_mesh* mesh;

CAT_vec4 eye;
CAT_mat4 V;
CAT_mat4 P;
CAT_mat4 PV;
CAT_mat4 S;

float theta_h;

void CAT_MS_hedron(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			mesh = &eth_mesh;

			eye = (CAT_vec4) {0, 0, 6, 1.0f};
			CAT_vec4 up = (CAT_vec4) {0, 1, 0, 0};
			CAT_vec4 forward = CAT_vec4_sub(eye, (CAT_vec4){0, 0, 0, 1.0f});
			CAT_vec4 right = CAT_vec4_cross(up, forward);
			up = CAT_vec4_cross(forward, right);
			right = CAT_vec4_normalize(right);
			up = CAT_vec4_normalize(up);
			forward = CAT_vec4_normalize(forward);
			float tx = CAT_vec4_dot(eye, right);
			float ty = CAT_vec4_dot(eye, up);
			float tz = CAT_vec4_dot(eye, forward);
			V = (CAT_mat4)
			{
				right.x, right.y, right.z, -tx,
				up.x, up.y, up.z, -ty,
				forward.x, forward.y, forward.z, -tz,
				0, 0, 0, 1
			};

			float n = 0.01f;
			float f = 100.0f;
			float hfov = 1.57079632679 * 0.5f;
			float width = 2 * n * tan(hfov / 2);
			float asp = ((float) LCD_SCREEN_W / (float) LCD_SCREEN_H);
			float height = width / asp;
			P = (CAT_mat4)
			{
				2 * n / width, 0, 0, 0,
				0, -2 * n / height, 0, 0,
				0, 0, -f/(f-n), -f*n/(f-n),
				0, 0, -1, 0
			};

			PV = CAT_matmul(P, V);

			S = (CAT_mat4)
			{
				LCD_SCREEN_W / 2, 0, 0, LCD_SCREEN_W / 2,
				0, LCD_SCREEN_H / 2, 0, LCD_SCREEN_H / 2,
				0, 0, 1, 0,
				0, 0, 0, 1
			};

			theta_h = 0;
			break;
		case CAT_MACHINE_SIGNAL_TICK:
			if(CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_back();

			theta_h += CAT_get_delta_time();
			if(theta_h >= 6.28318530718f)
				theta_h = 0;
			break;
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

CAT_vec4 fvtov4(int fidx, int vidx)
{
	return (CAT_vec4)
	{
		mesh->verts[mesh->faces[fidx*3+vidx]*3+0],
		mesh->verts[mesh->faces[fidx*3+vidx]*3+1],
		mesh->verts[mesh->faces[fidx*3+vidx]*3+2],
		1.0f
	};
}

void CAT_render_hedron()
{
	CAT_frameberry(0x0000);

	CAT_mat4 M = CAT_rotmat(0, theta_h, 0);
	CAT_mat4 MVP = CAT_matmul(PV, M);

	// The train arrives in clipspace
	for(int i = 0; i < mesh->n_faces; i++)
	{
		CAT_vec4 a = fvtov4(i, 0);
		CAT_vec4 b = fvtov4(i, 1);
		CAT_vec4 c = fvtov4(i, 2);

		a = CAT_matvec_mul(MVP, a);
		b = CAT_matvec_mul(MVP, b);
		c = CAT_matvec_mul(MVP, c);
	
		// W-culling
		bool clip_a = CAT_is_clipped(a);
		bool clip_b = CAT_is_clipped(b);
		bool clip_c = CAT_is_clipped(c);
		if(clip_a && clip_b && clip_c)
			continue;

		CAT_perspdiv(&a);
		CAT_perspdiv(&b);
		CAT_perspdiv(&c);

		a = CAT_matvec_mul(S, a);
		b = CAT_matvec_mul(S, b);
		c = CAT_matvec_mul(S, c);
		
		CAT_lineberry(a.x, a.y, b.x, b.y, 0xFFFF);
		CAT_lineberry(b.x, b.y, c.x, c.y, 0xFFFF);
		CAT_lineberry(c.x, c.y, a.x, a.y, 0xFFFF);
	}
}

static CAT_button konami_spell[10] =
{
	CAT_BUTTON_UP,
	CAT_BUTTON_UP,
	CAT_BUTTON_DOWN,
	CAT_BUTTON_DOWN,
	CAT_BUTTON_LEFT,
	CAT_BUTTON_RIGHT,
	CAT_BUTTON_LEFT,
	CAT_BUTTON_RIGHT,
	CAT_BUTTON_B,
	CAT_BUTTON_A
};

void CAT_MS_magic(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			CAT_input_buffer_clear();
			break;
		case CAT_MACHINE_SIGNAL_TICK:
			if(CAT_input_held(CAT_BUTTON_B, 0.5f))
				CAT_machine_back();

			if(CAT_input_spell(konami_spell))
				CAT_machine_transition(CAT_MS_hedron);
			break;
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_magic()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
	CAT_gui_text("MAGIC ");

	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18}); 
	CAT_gui_text("Enter an incantation,\n");
	CAT_gui_text("or hold ");
	CAT_gui_image(icon_b_sprite, 0);
	CAT_gui_text(" to exit.");
	
	CAT_gui_div("INCANTATION");
	int i = (input.buffer_head+9) % 10;
	int steps = 0;
	while(steps < 10)
	{
		if(input.buffer[i] != CAT_BUTTON_LAST)
			CAT_gui_image(icon_input_sprite, input.buffer[i]);
		i -= 1;
		if(i < 0)
			i = 9;
		steps += 1;
	}
}
