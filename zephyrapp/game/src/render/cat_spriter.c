#include "cat_render.h"

#include "sprite_assets.h"

//////////////////////////////////////////////////////////////////////////
// SPRITER

static CAT_draw_flag draw_flags = CAT_DRAW_FLAG_DEFAULT;
static bool draw_flags_set = false;
static uint16_t draw_colour = CAT_WHITE;
static bool draw_colour_set = false;
static unsigned int draw_scale = 1;
static bool draw_scale_set = false;

void CAT_push_draw_flags(int flags)
{
	draw_flags = flags;
	draw_flags_set = true;
}

void CAT_push_draw_colour(uint16_t colour)
{
	draw_colour = colour;
	draw_colour_set = true;
}

void CAT_push_draw_scale(unsigned int scale)
{
	draw_scale = scale;
	draw_scale_set = true;
}

void CAT_draw_sprite(const CAT_sprite* sprite, int frame_idx, int x, int y)
{
	CAT_draw_flag flags_override = draw_flags;
	uint16_t colour_override = draw_colour;
	int scale_override = draw_scale;

	if(sprite == NULL)
	{
		CAT_printf("[WARNING] CAT_draw_sprite: null sprite\n");
		sprite = &null_sprite;
		frame_idx = 0;
	}
	else if(frame_idx == -1)
	{
		frame_idx = CAT_animator_get_frame(sprite);
	}
	else if(frame_idx < 0 || frame_idx >= sprite->frame_count)
	{
		CAT_printf("[WARNING] CAT_draw_sprite: frame index %d out of bounds\n", frame_idx);
		frame_idx = sprite->frame_count-1;
	}

	uint16_t* framebuffer = CAT_LCD_get_framebuffer();

	int read_width = sprite->width;
	int read_height = sprite->height;
	int write_width = sprite->width * (draw_scale_set ? scale_override : 1);
	int write_height = sprite->height * (draw_scale_set ? scale_override : 1);

	if(draw_flags_set)
	{
		if ((flags_override & CAT_DRAW_FLAG_CENTER_X) > 0)
			x -= write_width / 2;
		if ((flags_override & CAT_DRAW_FLAG_CENTER_Y) > 0)
			y -= write_height / 2;
		else if ((flags_override & CAT_DRAW_FLAG_BOTTOM) > 0)
			y -= write_height;
	}
	
	y -= FRAMEBUFFER_ROW_OFFSET;
	int y_f = y + write_height;
	if (y >= CAT_LCD_FRAMEBUFFER_H)
		goto draw_sprite_end;
	if (y_f < 0)
		goto draw_sprite_end;
	if (y_f > CAT_LCD_FRAMEBUFFER_H)
		y_f = CAT_LCD_FRAMEBUFFER_H;	

	const uint8_t* frame = sprite->frames[frame_idx];
	int run_idx = 0;
	int dy = 0;
	int dx = 0;
	bool valid_draw_region = true;

	while(valid_draw_region)
	{
		uint8_t token = frame[run_idx++];
		uint16_t colour_idx = token == 0xff ? frame[run_idx++] : token;
		uint16_t colour = sprite->colour_table[colour_idx];
		if(colour != 0xdead)
		{
			if(draw_colour_set)
				colour = ADAPT_DESKTOP_COLOUR(colour_override);
			else
				colour = ADAPT_EMBEDDED_COLOUR(colour);
		}
		uint8_t run_length = token == 0xff ? frame[run_idx++] : 1;
		uint8_t run_remainder = run_length;
		
		while(run_remainder > 0)
		{
			if(draw_scale_set)
			{
				for(int i = 0; i < scale_override; i++)
				{
					int y_w = y + dy * scale_override + i;

					for(int j = 0; j < scale_override; j++)
					{
						int x_w =
						((draw_flags & CAT_DRAW_FLAG_REFLECT_X) > 0) ?
						x + (read_width - dx) * scale_override - j :
						x + dx * scale_override + j;

						if
						(
							colour != 0xdead &&
							y_w >= 0 && y_w < CAT_LCD_FRAMEBUFFER_H &&
							x_w >= 0 && x_w < CAT_LCD_FRAMEBUFFER_W
						)
						{
							int px_idx = y_w * CAT_LCD_FRAMEBUFFER_W + x_w;
							framebuffer[px_idx] = colour;
						}
					}
				}
			}
			else
			{
				int y_w = y + dy;
				int x_w =
				((draw_flags & CAT_DRAW_FLAG_REFLECT_X) > 0) ?
				x + read_width - dx :
				x + dx;

				if
				(
					colour != 0xdead &&
					y_w >= 0 && y_w < CAT_LCD_FRAMEBUFFER_H &&
					x_w >= 0 && x_w < CAT_LCD_FRAMEBUFFER_W
				)
				{
					int px_idx = y_w * CAT_LCD_FRAMEBUFFER_W + x_w;
					framebuffer[px_idx] = colour;
				}
			}
			
			dx += 1;
			if (dx >= read_width)
			{
				dx = 0;
				dy += 1;
				if(dy >= read_height || (y + dy) >= y_f)
				{
					valid_draw_region = false;
					break;
				}
			}
			run_remainder -= 1;
		}
	}

draw_sprite_end:
	draw_flags_set = false;
	draw_colour_set = false;
	draw_scale_set = false;
	return;
}