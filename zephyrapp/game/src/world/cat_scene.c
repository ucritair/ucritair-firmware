#include "cat_scene.h"
#include "cat_gui.h"

void viewport_transform(int x, int y, int eye_x, int eye_y, int view_w, int view_h, int* x_out, int* y_out)
{
	*x_out = x - eye_x + CAT_LCD_SCREEN_W/2;
	*y_out = y - eye_y + CAT_LCD_SCREEN_H/2;
}

void CAT_render_scene(CAT_scene* scene, int eye_x, int eye_y)
{
	for(int i = 0; i < scene->layer_count; i++)
	{
		struct layer* layer = &scene->layers[i];
		for(int j = 0; j < layer->prop_count; j++)
		{
			int x0, y0, x1, y1;

			struct prop* prop = &layer->props[j];
			viewport_transform(prop->position_x, prop->position_y, eye_x, eye_y, CAT_LCD_SCREEN_W, CAT_LCD_SCREEN_H, &x0, &y0);
			CAT_draw_sprite_raw(prop->prop->sprite, -1, x0, y0);

			for(int k = 0; k < prop->prop->blocker_count; k++)
			{
				x0 = prop->prop->blockers[k][0] + prop->position_x;
				y0 = prop->prop->blockers[k][1] + prop->position_y;
				x1 = prop->prop->blockers[k][2] + prop->position_x;
				y1 = prop->prop->blockers[k][3] + prop->position_y;
				viewport_transform(x0, y0, eye_x, eye_y, CAT_LCD_SCREEN_W, CAT_LCD_SCREEN_H, &x0, &y0);
				viewport_transform(x1, y1, eye_x, eye_y, CAT_LCD_SCREEN_W, CAT_LCD_SCREEN_H, &x1, &y1);
				CAT_strokeberry(x0, y0, x1-x0, y1-y0, CAT_RED);
			}

			for(int k = 0; k < prop->prop->trigger_count; k++)
			{
				x0 = prop->prop->triggers[k].aabb[0] + prop->position_x;
				y0 = prop->prop->triggers[k].aabb[1] + prop->position_y;
				x1 = prop->prop->triggers[k].aabb[2] + prop->position_x;
				y1 = prop->prop->triggers[k].aabb[3] + prop->position_y;
				viewport_transform(x0, y0, eye_x, eye_y, CAT_LCD_SCREEN_W, CAT_LCD_SCREEN_H, &x0, &y0);
				viewport_transform(x1, y1, eye_x, eye_y, CAT_LCD_SCREEN_W, CAT_LCD_SCREEN_H, &x1, &y1);
				CAT_strokeberry(x0, y0, x1-x0, y1-y0, CAT_GREEN);
			}
		}
	}
}