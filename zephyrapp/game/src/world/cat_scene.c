#include "cat_scene.h"
#include "cat_gui.h"

void CAT_render_scene(CAT_scene* scene, int eye_x, int eye_y)
{
	eye_x -= CAT_LCD_SCREEN_W/2;
	eye_y -= CAT_LCD_SCREEN_H/2;

	for(int i = 0; i < scene->layer_count; i++)
	{
		struct layer* layer = &scene->layers[i];
		for(int j = 0; j < layer->prop_count; j++)
		{
			struct prop* prop = &layer->props[j];
			int x = prop->position_x - eye_x;
			int y = prop->position_y - eye_y;
			CAT_draw_sprite_raw(prop->prop->sprite, -1, x, y);
		}
	}
}