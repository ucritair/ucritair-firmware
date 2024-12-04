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
	{"DEBUG", CAT_MS_debug},
	{"CHEATS", CAT_MS_cheats},
	{"SOUND", CAT_MS_sound},
	{"KONAMI", CAT_MS_konami},
	{"MANUAL", CAT_MS_manual},
	{"BACK", CAT_MS_room}
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
	CAT_gui_text_wrap(flavour_text[flavour_idx]);
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

#include "cat_mesh_asset.h"
#define NUM_FACES (sizeof(mesh.faces) / sizeof(mesh.faces[0]))

CAT_vec4 eye;
CAT_mat4 V;
CAT_mat4 P;
CAT_mat4 PV;
CAT_mat4 S;

float theta_h;

bool wireframe;

void CAT_MS_hedron(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
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

			wireframe = false;
			break;
		case CAT_MACHINE_SIGNAL_TICK:
			if(CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_back();

			if(CAT_input_pressed(CAT_BUTTON_SELECT))
				wireframe = !wireframe;

			theta_h += CAT_get_delta_time();
			if(theta_h >= 6.28318530718f)
				theta_h = 0;
			break;
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_hedron()
{
	CAT_frameberry(0x0000);
	CAT_depthberry();

	CAT_mat4 M = CAT_rotmat(0, theta_h, 0);
	CAT_mat4 MV = CAT_matmul(V, M);
	CAT_mat4 MVP = CAT_matmul(PV, M);

	// The train arrives in clipspace
	for(int i = 0; i < NUM_FACES; i++)
	{
		CAT_vec4 a = mesh.verts[mesh.faces[i][0]];
		CAT_vec4 b = mesh.verts[mesh.faces[i][1]];
		CAT_vec4 c = mesh.verts[mesh.faces[i][2]];

		a = CAT_matvec_mul(MVP, a);
		b = CAT_matvec_mul(MVP, b);
		c = CAT_matvec_mul(MVP, c);
	
		// W-culling
		bool clip_a = CAT_is_clipped(a);
		bool clip_b = CAT_is_clipped(b);
		bool clip_c = CAT_is_clipped(c);
		if(clip_a && clip_b && clip_c)
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

		// LIGHTING	
#ifdef CAT_DESKTOP
		CAT_vec4 p1 = mesh.verts[mesh.faces[i][0]];
		CAT_vec4 p2 = mesh.verts[mesh.faces[i][1]];
		CAT_vec4 p3 = mesh.verts[mesh.faces[i][2]];
		p1 = CAT_matvec_mul(MV, p1);
		p2 = CAT_matvec_mul(MV, p2);
		p3 = CAT_matvec_mul(MV, p3);
		float amb = 0.05f;
		CAT_vec4 centroid = CAT_centroid(p1, p2, p3);
		CAT_vec4 L = CAT_vec4_normalize(CAT_vec4_sub(eye, centroid));
		CAT_vec4 N = CAT_vec4_normalize(CAT_vec4_cross(CAT_vec4_sub(p2, p1), CAT_vec4_sub(p3, p1)));
		float diff = clampf(CAT_vec4_dot(N, L), 0.0f, 1.0f);
		uint8_t light = 255 * clampf(amb + diff, 0.0f, 1.0f) * 0.75f;
#else
		uint8_t light = 255;
#endif
		
		if(wireframe)
		{
			CAT_lineberry(a.x, a.y, b.x, b.y, 0xFFFF);
			CAT_lineberry(b.x, b.y, c.x, c.y, 0xFFFF);
			CAT_lineberry(c.x, c.y, a.x, a.y, 0xFFFF);
		}
		else
		{
			CAT_triberry
			(
				a.x, a.y, a.z,
				b.x, b.y, b.z,
				c.x, c.y, c.z,
				RGB8882565(light, light, light)
			);
		}
	}
}

#ifdef CAT_EMBEDDED
#include "cat_sound_asset.h"
#include "sound.h"
#endif

void CAT_MS_sound(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
#ifdef CAT_EMBEDDED
			soundPower(true);
#endif
			break;
		case CAT_MACHINE_SIGNAL_TICK:
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_back();
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_room);
#ifdef CAT_EMBEDDED
			if(CAT_input_pulse(CAT_BUTTON_A))
				soundPlay(sound.samples, sizeof(sound.samples), SoundReplaceCurrent);
#endif
			break;
		case CAT_MACHINE_SIGNAL_EXIT:
#ifdef CAT_EMBEDDED
			soundPower(false);
#endif
			break;
	}
}

void CAT_render_sound()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 20});
#ifdef CAT_EMBEDDED
	CAT_gui_text("Press A for sound\n");
	if(soundIsPlaying())
	{
		CAT_gui_text("Sound is playing\n");
		CAT_gui_image(cliff_racer_sprite, 0);
	}
#else
	CAT_gui_text_wrap("Sound is only available on embedded!\n");
#endif
}

void CAT_MS_konami(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			break;
		case CAT_MACHINE_SIGNAL_TICK:
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_back();

			if(CAT_konami())
				CAT_machine_transition(CAT_MS_hedron);
			break;
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_konami()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 20});
	CAT_gui_text_wrap("Enter the Konami code to continue, or press START to go back\n");
}
