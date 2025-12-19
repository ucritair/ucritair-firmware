#include "cat_main.h"

#include "cat_core.h"
#include "cat_save.h"
#include "cat_render.h"
#include "cat_air.h"
#include "cat_persist.h"
#include "cat_room.h"
#include "cat_monitors.h"
#include "cat_gui.h"
#include "cat_effects.h"
#include "cat_spriter.h"
#include "cat_pet.h"
#include "cat_input.h"
#include "cat_wifi.h"
#include "msht.h"
#include "cat_chat.h"
#include "cat_radio.h"
#include "cat_crypto.h"
#include "cat_screen_saver.h"

#include "sprite_assets.h"
#include "tinysprite_assets.h"

void CAT_init()
{
	CAT_platform_init();

	CAT_wifi_init();
	CAT_wifi_autoconnect(1000);

	CAT_radio_init();
	CAT_radio_clear_buffer();
	CAT_radio_start_modem();
	CAT_radio_add_chanels();
	
	CAT_input_init();
	CAT_rand_seed();
	CAT_animator_init();

	CAT_pet_init();
	CAT_room_init();

	CAT_force_load();

	/*if(CAT_check_save_flags(CAT_SAVE_CONFIG_FLAG_RESEARCH))
	{
		CAT_pushdown_rebase(CAT_MS_screen_saver);
	}
	else if(persist_flags & CAT_PERSIST_CONFIG_FLAG_AQ_FIRST)
	{
		CAT_pushdown_rebase(CAT_MS_monitor);
	}
	else
		CAT_pushdown_rebase(CAT_MS_room);*/

#ifdef CAT_RESEARCH_ONLY
	CAT_pushdown_rebase(CAT_MS_screen_saver);
#else
	if(persist_flags & CAT_PERSIST_CONFIG_FLAG_AQ_FIRST)
		CAT_pushdown_rebase(CAT_MS_monitor);
	else
		CAT_pushdown_rebase(CAT_MS_room);
#endif
}

void CAT_tick_logic()
{	
	if(CAT_check_load_flags(CAT_LOAD_FLAG_DIRTY))
	{
		CAT_force_load();
		CAT_unset_load_flags(CAT_LOAD_FLAG_DIRTY);
	}
		
	CAT_platform_tick();
	CAT_input_tick();

	CAT_radio_poll_RX(CAT_chat_RX_meowback);

	CAT_AQ_tick();
	CAT_AQ_crisis_tick();

	CAT_animator_tick();
	CAT_room_tick();
	CAT_pet_tick();
	
	CAT_pushdown_tick();

	CAT_gui_tick();
	CAT_effects_tick();

	CAT_orientation_tick();

	if(CAT_eink_should_update())
		CAT_set_eink_update_flag(true);
}

void CAT_draw_eink_refresh_notice()
{
	CAT_fillberry(0, 0, CAT_LCD_SCREEN_W, CAT_LCD_SCREEN_H/2, CAT_SKY_BLUE);
	CAT_draw_background(&monitor_clouds_sprite, 0, CAT_LCD_SCREEN_H/2-monitor_clouds_sprite.height);

	for(int y = CAT_LCD_SCREEN_H/2; y < CAT_LCD_SCREEN_H-48; y += CAT_TILE_SIZE)
	{
		for(int x = 0; x < CAT_LCD_SCREEN_W; x += CAT_TILE_SIZE)
		{
			CAT_draw_tile(&floor_roof_tile_sprite, 2, x, y);
		}
	}

	CAT_draw_sprite_raw(AS_idle.tick_sprite, 0, 16*5+8, CAT_LCD_SCREEN_H/2-12);
	CAT_draw_sprite_raw(&eink_update_machine_sprite, 0, 16*9, CAT_LCD_SCREEN_H/2-eink_update_machine_sprite.height+16*2);
	CAT_draw_sprite_raw(&eink_update_pen_sprite, 0, 16*3+8, CAT_LCD_SCREEN_H/2-4);
	CAT_draw_sprite_raw(&eink_update_clipboard_sprite, 0, 16*4-4, CAT_LCD_SCREEN_H/2+12);

	CAT_fillberry(0, CAT_LCD_SCREEN_H-48, CAT_LCD_SCREEN_W, 48, CAT_RGB8882565(105, 79, 98));

	CAT_set_text_colour(CAT_WHITE);
	CAT_draw_text(8, 8, "UPDATING E-INK DISPLAY...");

	CAT_set_text_flags(CAT_TEXT_FLAG_CENTER);
	CAT_set_text_scale(2);
	CAT_set_text_colour(CAT_WHITE);
	CAT_draw_text(CAT_LCD_SCREEN_W/2, CAT_LCD_SCREEN_H-36, "PLEASE WAIT");
}

void CAT_draw_splash()
{
	CAT_frameberry(CAT_BLACK);
	CAT_draw_tinysprite(0, 0, &tnyspr_ucrit_splash, CAT_WHITE, CAT_BLACK);
}

void CAT_tick_render()
{
	if(CAT_is_first_render_cycle())
		CAT_flip_render_callback();

	CAT_render_callback render_callback = CAT_get_render_callback();
	if(render_callback != NULL)
	{
		render_callback();
	}
	else
	{
		/*
		Effortlessly at height hangs his still eye.
		His wings hold all creation in a weightless quiet,
		steady as a hallucination in the streaming air.
		*/
		CAT_set_sprite_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y);
		CAT_draw_sprite(&null_sprite, 0, 120, 160);
	}

	CAT_gui_render();
	CAT_effects_render();

	if(CAT_poll_eink_update_flag())
	{
		if(CAT_eink_is_boot_update())
			CAT_draw_splash();
		else
			CAT_draw_eink_refresh_notice();
	}
}

#ifdef CAT_DESKTOP
#include <unistd.h>

int main(int argc, char** argv)
{	
	CAT_init();
	
	while (CAT_is_on())
	{
		CAT_tick_logic();

		for(int render_cycle = 0; render_cycle < CAT_LCD_FRAMEBUFFER_SEGMENTS; render_cycle++)
		{
			CAT_set_render_cycle(render_cycle);
			CAT_tick_render();
			CAT_LCD_post();
		}
		CAT_LCD_flip();

		if(CAT_poll_eink_update_flag())
			CAT_eink_execute_update();

		if(CAT_get_battery_pct() == 0)
			CAT_shutdown();

		// 1 / FPS * 1_000_000 yields microseconds between framebuffer updates at fixed FPS
		usleep((1.0f / 14.0f) * 1000000);
	}

	CAT_force_save();
	CAT_platform_cleanup();
	return 0;
}
#endif