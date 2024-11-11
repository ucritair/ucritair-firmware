#include "cat_stats.h"
#include "cat_room.h"
#include "cat_item.h"
#include "cat_gui.h"
#include "cat_input.h"
#include "cat_pet.h"
#include "cat_sprite.h"

void CAT_MS_stats(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			break;
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_transition(&machine, CAT_MS_menu);
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(&machine, CAT_MS_room);
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_stats()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
	CAT_gui_text("INSIGHTS ");
	CAT_gui_image(icon_b_sprite, 1);
	CAT_gui_image(icon_exit_sprite, 0);

	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});  
	CAT_gui_image(pet_idle_sprite, 0); 
	
	CAT_gui_div("CORE STATS");

	CAT_gui_image(icon_vig_sprite, 0);
	CAT_gui_text("VIG ");
	for(int i = 1; i <= 12; i++)
		CAT_gui_image(i <= pet.vigour ? cell_vig_sprite : cell_empty_sprite, 0);
	CAT_gui_line_break();

	CAT_gui_image(icon_foc_sprite, 0);
	CAT_gui_text("FOC ");
	for(int i = 1; i <= 12; i++)
		CAT_gui_image(i <= pet.focus ? cell_foc_sprite : cell_empty_sprite, 0);
	CAT_gui_line_break();

	CAT_gui_image(icon_spi_sprite, 0);
	CAT_gui_text("SPI ");
	for(int i = 1; i <= 12; i++)
		CAT_gui_image(i <= pet.spirit ? cell_spi_sprite : cell_empty_sprite, 0);

	CAT_gui_div("AIR QUALITY");
	
	int temp_idx, co2_idx, pm_idx, voc_idx, nox_idx;
	CAT_calc_quantized_aqi_scores(&temp_idx, &co2_idx, &pm_idx, &voc_idx, &nox_idx);
	CAT_gui_image(icon_temp_sprite, temp_idx);
	CAT_gui_image(icon_co2_sprite, co2_idx);
	CAT_gui_image(icon_pm_sprite, pm_idx);
	CAT_gui_image(icon_voc_sprite, voc_idx);
	CAT_gui_image(icon_nox_sprite, nox_idx);
	
	CAT_gui_div("INTERVENTIONS");
	if(CAT_gear_status(mask_item))
		CAT_gui_image(icon_mask_sprite, 0);
	if(CAT_room_find(purifier_item) != -1)
		CAT_gui_image(icon_pure_sprite, 0);
	if(CAT_room_find(uv_item) != -1)
		CAT_gui_image(icon_uv_sprite, 0);
}