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
#ifdef CAT_DESKTOP
	{"DEBUG", CAT_MS_debug},
	{"LITANY", CAT_MS_litany},
	{"CHEATS", CAT_MS_cheats},
	{"HEDRON", CAT_MS_hedron},
#endif
	{"MANUAL", CAT_MS_manual},
	{"BACK", CAT_MS_room}
};
#define NUM_MENU_ITEMS (sizeof(entries)/sizeof(entries[0]))

static int selector = 0;

void CAT_MS_menu(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
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
	CAT_gui_textf("Life: %0.0fs/%0.0fs\n", CAT_timer_get(pet.life_timer_id), timetable.durations[pet.life_timer_id]);
	CAT_gui_textf("Stat: %0.0fs/%0.0fs\n", CAT_timer_get(pet.stat_timer_id), timetable.durations[pet.stat_timer_id]);
	CAT_gui_textf("Earn: %0.0fs/%0.0fs\n", CAT_timer_get(room.earn_timer_id), timetable.durations[room.earn_timer_id]);
	CAT_gui_line_break();

	int occupied_spaces = 0;
	for(int y = 0; y < space.grid_shape.y; y++)
	{
		for(int x = 0; x < space.grid_shape.x; x++)
		{
			int idx = y * space.grid_shape.x + x;
			int cell = space.cells[idx];
			if(cell != 0)
				occupied_spaces += 1;
			CAT_gui_image(icon_cell_sprite, cell);
		}
		CAT_gui_line_break();
	}
	CAT_gui_textf
	(
		"%d occupied, %d free\n%d total",
		occupied_spaces, space.free_list_length,
		space.grid_shape.x * space.grid_shape.y
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

void CAT_render_litany()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 20});
	CAT_gui_text_wrap("THE LITANY AGAINST FEAR\nI must not fear. Fear is the mind-killer. Fear is the little-death that brings total obliteration.\nI will face my fear. I will permit it to pass over me and through me. And when it has gone past, I will turn the inner eye to see its path. Where the fear has gone there will be nothing. Only I will remain.");
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
		CAT_item_list_add(&bag, item_id);
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
				cheat_entries[cheat_selector].proc();
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
// You'll notice that I don't combine my transformations into one matrix yet.
// If the train takes you to clip-space too fast, you won't be able to see what's outside the window. Literally!

CAT_vec4 mesh[36] =
{
	{-0.5f, -0.5f, -0.5f, 1.0f},
	{0.5f, -0.5f, -0.5f, 1.0f},
	{0.5f,  0.5f, -0.5f, 1.0f},	
	{0.5f,  0.5f, -0.5f, 1.0f},
	{-0.5f,  0.5f, -0.5f, 1.0f},
	{-0.5f, -0.5f, -0.5f, 1.0f},

	{0.5f,  0.5f,  0.5f, 1.0f},
	{0.5f, -0.5f,  0.5f, 1.0f},
	{-0.5f, -0.5f,  0.5f, 1.0f},
	{-0.5f, -0.5f,  0.5f, 1.0f},
	{-0.5f,  0.5f,  0.5f, 1.0f},
	{0.5f,  0.5f,  0.5f, 1.0f},

	{-0.5f, -0.5f, -0.5f, 1.0f},
	{-0.5f,  0.5f, -0.5f, 1.0f},
	{-0.5f,  0.5f,  0.5f, 1.0f},
	{-0.5f,  0.5f,  0.5f, 1.0f},
	{-0.5f, -0.5f,  0.5f, 1.0f},
	{-0.5f, -0.5f, -0.5f, 1.0f},

	{0.5f, -0.5f, -0.5f, 1.0f},
	{0.5f, -0.5f,  0.5f, 1.0f},
	{0.5f,  0.5f,  0.5f, 1.0f},
	{0.5f,  0.5f,  0.5f, 1.0f},
	{0.5f,  0.5f, -0.5f, 1.0f},
	{0.5f, -0.5f, -0.5f, 1.0f},
	
	{0.5f, -0.5f,  0.5f, 1.0f},
	{0.5f, -0.5f, -0.5f, 1.0f},
	{-0.5f, -0.5f, -0.5f, 1.0f},
	{-0.5f, -0.5f, -0.5f, 1.0f},
	{-0.5f, -0.5f,  0.5f, 1.0f},
	{0.5f, -0.5f,  0.5f, 1.0f},

	{-0.5f,  0.5f, -0.5f, 1.0f},
	{0.5f,  0.5f, -0.5f, 1.0f},
	{0.5f,  0.5f,  0.5f, 1.0f},
	{0.5f,  0.5f,  0.5f, 1.0f},
	{-0.5f,  0.5f,  0.5f, 1.0f},
	{-0.5f,  0.5f, -0.5f, 1.0f}
};
#define NUM_VERTS (sizeof(mesh) / sizeof(mesh[0]))

float theta_h = 0;
float theta_v = 0;
float r = 2;
CAT_vec4 eye = {0};

void CAT_MS_hedron(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			theta_h = 0;
			theta_v = 0;
			r = 2;
			break;
		case CAT_MACHINE_SIGNAL_TICK:
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_back();

			if(CAT_input_held(CAT_BUTTON_LEFT, 0))
				theta_h += CAT_get_delta_time();
			if(CAT_input_held(CAT_BUTTON_RIGHT, 0))
				theta_h -= CAT_get_delta_time();
			if(CAT_input_held(CAT_BUTTON_UP, 0))
				theta_v += CAT_get_delta_time();
			if(CAT_input_held(CAT_BUTTON_DOWN, 0))
				theta_v -= CAT_get_delta_time();
			if(CAT_input_held(CAT_BUTTON_A, 0))
				r -= CAT_get_delta_time();
			if(CAT_input_held(CAT_BUTTON_B, 0))
				r += CAT_get_delta_time();
			r = clampf(r, 0, 10);

			break;
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
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

void CAT_render_hedron()
{
	CAT_frameberry(0x0000);

	CAT_vec4 verts[36];
	memcpy(verts, mesh, sizeof(mesh));

	CAT_mat4 M = CAT_rotmat(theta_v, theta_h, 0);
	
	eye = (CAT_vec4) {0, 0, r, 1.0f};
	CAT_vec4 forward = CAT_vec4_sub((CAT_vec4) {0}, eye);
	CAT_vec4 up = (CAT_vec4) {0, 1, 0, 0};
	CAT_vec4 right = CAT_vec4_cross(up, forward);
	up = CAT_vec4_cross(forward, right);
	right = CAT_vec4_normalize(right);
	up = CAT_vec4_normalize(up);
	forward = CAT_vec4_normalize(forward);
	float tx = -CAT_vec4_dot(eye, right);
	float ty = -CAT_vec4_dot(eye, up);
	float tz = -CAT_vec4_dot(eye, forward);
	CAT_mat4 V =
	{
		right.x, right.y, right.z, tx,
		up.x, up.y, up.z, ty,
		forward.x, forward.y, forward.z, tz,
		0, 0, 0, 1
	};

	float n = -0.001f;
	float f = -100.0f;
	float asp_inv = 1.0f / ((float) LCD_SCREEN_W / (float) LCD_SCREEN_H);
	float fov_inv = 1.0f / tan((2.094) / 2);
	CAT_mat4 P =
	{
		fov_inv * asp_inv, 0, 0, 0,
		0, fov_inv, 0, 0,
		0, 0, -(f+n)/(f-n), -2*f*n/(f-n),
		0, 0, -1, 0
	};
	
	CAT_mat4 S =
	{
		LCD_SCREEN_W, 0, 0, LCD_SCREEN_W / 2,
		0, LCD_SCREEN_H, 0, LCD_SCREEN_H / 2,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	for(int i = 0; i < NUM_VERTS; i++)
		verts[i] = CAT_matvec_mul(M, verts[i]);
	for(int i = 0; i < NUM_VERTS; i++)
		verts[i] = CAT_matvec_mul(V, verts[i]);
	for(int i = 0; i < NUM_VERTS; i++)
		verts[i] = CAT_matvec_mul(P, verts[i]);

	// The train arrives in clipspace
	for(int i = 0; i < NUM_VERTS; i += 3)
	{
		CAT_vec4 a = verts[i];
		CAT_vec4 b = verts[i+1];
		CAT_vec4 c = verts[i+2];

		// W-culling
		if(CAT_is_clipped(a))
			continue;
		if(CAT_is_clipped(b))
			continue;
		if(CAT_is_clipped(c))
			continue;

		// Backface culling
		CAT_vec4 ba = CAT_vec4_sub(b, a);
		CAT_vec4 ca = CAT_vec4_sub(c, a);
		CAT_vec4 norm = CAT_vec4_cross(ba, ca);
		float valign = CAT_vec4_dot(a, norm);
		if(valign >= 0)
			continue;

		CAT_perspdiv(&a);
		CAT_perspdiv(&b);
		CAT_perspdiv(&c);

		a = CAT_matvec_mul(S, a);
		b = CAT_matvec_mul(S, b);
		c = CAT_matvec_mul(S, c);

		CAT_bresenham(a.x, a.y, b.x, b.y, 0xFFFF);
		CAT_bresenham(b.x, b.y, c.x, c.y, 0xFFFF);
		CAT_bresenham(c.x, c.y, a.x, a.y, 0xFFFF);
	}
}
