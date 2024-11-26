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
	{"LITANY", CAT_MS_litany},
	{"CHEATS", CAT_MS_cheats},
	{"HEDRON", CAT_MS_hedron},
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
	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 20});

	CAT_gui_textf("Last sleep: %ds\n", logged_sleep);
	CAT_gui_textf("Life timer: %0.2fs\n", CAT_timer_get(pet.life_timer_id));
	CAT_gui_textf("Stat timer: %0.2fs\n", CAT_timer_get(pet.stat_timer_id));
	CAT_gui_textf("Earn timer: %0.2fs\n", CAT_timer_get(room.earn_timer_id));
	CAT_gui_line_break();

	for(int y = 0; y < space.grid_shape.y; y++)
	{
		for(int x = 0; x < space.grid_shape.x; x++)
		{
			int idx = y * space.grid_shape.x + x;
			int cell = space.cells[idx];
			CAT_gui_image(icon_cell_sprite, cell);
		}
		CAT_gui_line_break();
	}
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
	CAT_gui_text("THE LITANY AGAINST FEAR\nI must not fear. Fear is the mind-killer. Fear is the little-death that brings total obliteration.\nI will face my fear. I will permit it to pass over me and through me. And when it has gone past, I will turn the inner eye to see its path. Where the fear has gone there will be nothing. Only I will remain.");
}

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
			break;
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_cheats()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 20});
	CAT_gui_text("");
}

CAT_vec4 mesh[36] =
{
	{-0.5f, -0.5f, -0.5f, 1.0f},
	{0.5f, -0.5f, -0.5f, 1.0f},
	{0.5f,  0.5f, -0.5f, 1.0f},
	{0.5f,  0.5f, -0.5f, 1.0f},
	{-0.5f,  0.5f, -0.5f, 1.0f},
	{-0.5f, -0.5f, -0.5f, 1.0f},

	{-0.5f, -0.5f,  0.5f, 1.0f},
	{0.5f, -0.5f,  0.5f, 1.0f},
	{0.5f,  0.5f,  0.5f, 1.0f},
	{0.5f,  0.5f,  0.5f, 1.0f},
	{-0.5f,  0.5f,  0.5f, 1.0f},
	{-0.5f, -0.5f,  0.5f, 1.0f},

	{-0.5f,  0.5f,  0.5f, 1.0f},
	{-0.5f,  0.5f, -0.5f, 1.0f},
	{-0.5f, -0.5f, -0.5f, 1.0f},
	{-0.5f, -0.5f, -0.5f, 1.0f},
	{-0.5f, -0.5f,  0.5f, 1.0f},
	{-0.5f,  0.5f,  0.5f, 1.0f},

	{0.5f,  0.5f,  0.5f, 1.0f},
	{0.5f,  0.5f, -0.5f, 1.0f},
	{0.5f, -0.5f, -0.5f, 1.0f},
	{0.5f, -0.5f, -0.5f, 1.0f},
	{0.5f, -0.5f,  0.5f, 1.0f},
	{0.5f,  0.5f,  0.5f, 1.0f},

	{-0.5f, -0.5f, -0.5f, 1.0f},
	{0.5f, -0.5f, -0.5f, 1.0f},
	{0.5f, -0.5f,  0.5f, 1.0f},
	{0.5f, -0.5f,  0.5f, 1.0f},
	{-0.5f, -0.5f,  0.5f, 1.0f},
	{-0.5f, -0.5f, -0.5f, 1.0f},

	{-0.5f,  0.5f, -0.5f, 1.0f},
	{0.5f,  0.5f, -0.5f, 1.0f},
	{0.5f,  0.5f,  0.5f, 1.0f},
	{0.5f,  0.5f,  0.5f, 1.0f},
	{-0.5f,  0.5f,  0.5f, 1.0f},
	{-0.5f,  0.5f, -0.5f, 1.0f}
};
#define NUM_VERTS (sizeof(mesh) / sizeof(mesh[0]))

static CAT_vec4 translation = {0, 0, -2, 1};

void CAT_MS_hedron(CAT_machine_signal signal)
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
			
			if(CAT_input_held(CAT_BUTTON_LEFT, 0))
				translation.x += CAT_get_delta_time();
			if(CAT_input_held(CAT_BUTTON_RIGHT, 0))
				translation.x -= CAT_get_delta_time();
			if(CAT_input_held(CAT_BUTTON_UP, 0))
				translation.z += CAT_get_delta_time();
			if(CAT_input_held(CAT_BUTTON_DOWN, 0))
				translation.z -= CAT_get_delta_time();
			break;
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_hedron()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 20});

	CAT_vec4 verts[36];
	memcpy(verts, mesh, sizeof(mesh));
	
	

	CAT_mat4 V =
	{
		1, 0, 0, -translation.x,
		0, 1, 0, -translation.y,
		0, 0, 1, -translation.z,
		0, 0, 0, 1
	};
	for(int i = 0; i < NUM_VERTS; i++)
		verts[i] = CAT_matvec_mul(V, verts[i]);

	float n = 0.01f;
	float f = -100.0f;
	CAT_mat4 P =
	{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, -f/(f-n), -(f*n)/(f-n),
		0, 0, -1, 0
	};
	for(int i = 0; i < NUM_VERTS; i++)
		verts[i] = CAT_matvec_mul(P, verts[i]);

	float aspect = (float) LCD_SCREEN_W / (float) LCD_SCREEN_H;
	CAT_mat4 S =
	{
		LCD_SCREEN_W, 0, 0, LCD_SCREEN_W / 2,
		0, LCD_SCREEN_H * aspect, 0, LCD_SCREEN_H / 2,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	for(int i = 0; i < NUM_VERTS; i++)
		verts[i] = CAT_matvec_mul(S, verts[i]);

	// The train arrives in clipspace
	for(int i = 0; i < NUM_VERTS-3; i += 3)
	{
		CAT_vec4 a = verts[i];
		CAT_vec4 b = verts[i+1];
		CAT_vec4 c = verts[i+2];

		if(a.w >= 0 || b.w >= 0 || c.w >= 0)
			continue;
		CAT_vec4 norm = CAT_vec4_cross(a, b);
		float ndv = CAT_vec4_dot(norm, (CAT_vec4) {0, 0, -1, 0});
		if(ndv < 0)
			continue;

		CAT_perspdiv(&a);
		CAT_perspdiv(&b);
		CAT_perspdiv(&c);

		CAT_bresenham(a.x, a.y, b.x, b.y, 0x0000);
		CAT_bresenham(b.x, b.y, c.x, c.y, 0x0000);
		CAT_bresenham(c.x, c.y, a.x, a.y, 0x0000);
	}
}

