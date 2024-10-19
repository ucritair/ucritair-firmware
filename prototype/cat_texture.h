#ifndef CAT_TEXTURE_H
#define CAT_TEXTURE_H

#include "png.h"

#include "cat_math.h"

typedef struct CAT_colour
{
	float r;
	float g;
	float b;
	float a;
} CAT_colour;

CAT_colour CAT_colour_lerp(CAT_colour a, CAT_colour b, float t)
{
	CAT_colour c;
	c.r = lerp(a.r, b.r, t);
	c.g = lerp(a.g, b.g, t);
	c.b = lerp(a.b, b.b, t);
	c.a = lerp(a.a, b.a, t);
	return c;
}

typedef struct CAT_texture
{
	int width;
	int height;
	float* data;
} CAT_texture;

void CAT_texture_init(CAT_texture* tex, int width, int height)
{
	tex->width = width;
	tex->height = height;
	tex->data = (float*) malloc(width * height * 4 * sizeof(float));
}

CAT_colour CAT_texture_read(CAT_texture* tex, int x, int y)
{
	if
	(
		x < 0 || x >= tex->width ||
		y < 0 || y >= tex->height
	)
	{
		return {0, 0, 0, 0};
	}

	int idx = (y * tex->width + x) * 4;
	CAT_colour c;
	c.r = tex->data[idx+0];
	c.g = tex->data[idx+1];
	c.b = tex->data[idx+2];
	c.a = tex->data[idx+3];
	return c;
}

void CAT_texture_write(CAT_texture* tex, int x, int y, CAT_colour c)
{
	if
	(
		x < 0 || x >= tex->width ||
		y < 0 || y >= tex->height
	)
	{
		return;
	}

	int idx = (y * tex->width + x) * 4;
	CAT_colour base = CAT_texture_read(tex, x, y);
	CAT_colour blend = CAT_colour_lerp(base, c, c.a);
	tex->data[idx+0] = blend.r;
	tex->data[idx+1] = blend.g;
	tex->data[idx+2] = blend.b;
	tex->data[idx+3] = blend.a;
}

void CAT_load_PNG(CAT_texture* tex, const char* path)
{
	FILE* file = fopen(path, "rb");

	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	png_infop info = png_create_info_struct(png);
	png_init_io(png, file);
	png_set_sig_bytes(png, 0);
	int png_transforms =
		PNG_TRANSFORM_STRIP_16 |
		PNG_TRANSFORM_PACKING |
		PNG_TRANSFORM_EXPAND;
	png_read_png(png, info, png_transforms, NULL);

	png_uint_32 width;
	png_uint_32 height;
	png_get_IHDR(png, info, &width, &height, NULL, NULL, NULL, NULL, NULL);\
	
	size_t row_size = png_get_rowbytes(png, info);
	size_t size = row_size * height;
	uint8_t bytes[size];
	
	png_bytepp rows = png_get_rows(png, info);
	for(int i = 0; i < height; i++)
	{
		memcpy(bytes + row_size * i, rows[i], row_size);
	}

	CAT_texture_init(tex, width, height);
	for(int y = 0; y < height*4; y++)
	{
		for(int x = 0; x < width*4; x++)
		{
			int idx = y * width + x;
			uint8_t c_b = bytes[idx];
			float c_f = c_b / 255.0f;
			tex->data[idx] = c_f;
		}
	}

	png_destroy_read_struct(&png, &info, NULL);
}

typedef struct CAT_sprite
{
	int x;
	int y;
	int width;
	int height;
} CAT_sprite;

typedef struct CAT_atlas
{
	CAT_texture* tex;
	CAT_sprite map[256];
} CAT_atlas;

void CAT_atlas_init(CAT_atlas* atlas, CAT_texture* tex)
{
	atlas->tex = tex;
	for(int i = 0; i < 256; i++)
	{
		atlas->map[i] = {0, 0, 0, 0};
	}
}

void CAT_atlas_add(CAT_atlas* atlas, int key, CAT_sprite sprite)
{
	atlas->map[key] = sprite;
}

typedef enum CAT_draw_mode
{
	CAT_DRAW_MODE_DEFAULT = 0,
	CAT_DRAW_MODE_BOTTOM = 1,
	CAT_DRAW_MODE_CENTER_X = 2,
	CAT_DRAW_MODE_CENTER_Y = 4,
	CAT_DRAW_MODE_REFLECT_X = 8
} CAT_draw_mode;

void CAT_draw_sprite(CAT_texture* frame, int x, int y, CAT_atlas* atlas, int key, int mode)
{
	CAT_sprite sprite = atlas->map[key];
	
	int x_shift = 0;
	int y_shift = 0;
	if((mode & CAT_DRAW_MODE_BOTTOM) > 0)
		y_shift = -sprite.height;
	if((mode & CAT_DRAW_MODE_CENTER_X) > 0)
		x_shift = -sprite.width/2;
	if((mode & CAT_DRAW_MODE_CENTER_Y) > 0)
		y_shift = -sprite.height/2;

	for(int dy = 0; dy < sprite.height; dy++)
	{
		for(int dx = 0; dx < sprite.width; dx++)
		{
			int x_r = sprite.x+dx;
			if((mode & CAT_DRAW_MODE_REFLECT_X) > 0)
				x_r = sprite.x+sprite.width-1-dx;
			int y_r = sprite.y+dy;
			CAT_colour c = CAT_texture_read(atlas->tex, x_r, y_r);
			int x_w = x+dx+x_shift;
			int y_w = y+dy+y_shift;
			CAT_texture_write(frame, x_w, y_w, c);
		}
	}
}

typedef struct CAT_anim
{
	int frame_count;
	int frames[24];
	int idx;
	int looping;
} CAT_anim;

void CAT_anim_init(CAT_anim* anim)
{
	anim->frame_count = 0;
	anim->idx = 0;
	anim->looping = 1;
}

void CAT_anim_add(CAT_anim* anim, int key)
{
	anim->frames[anim->frame_count] = key;
	anim->frame_count += 1;
}

void CAT_anim_tick(CAT_anim* anim)
{
	anim->idx += 1;
	if(anim->looping && anim->idx >= anim->frame_count)
	{
		anim->idx = 0;
	}
}

typedef struct CAT_anim_command
{
	CAT_anim* anim;
	int layer;
	int x;
	int y;
	int mode;
} CAT_anim_command;

void CAT_anim_command_init(CAT_anim_command* command, CAT_anim* anim, int layer, int x, int y, int mode)
{
	command->anim = anim;
	command->layer = layer;
	command->x = x;
	command->y = y;
	command->mode = mode;
}

typedef struct CAT_anim_queue
{
	CAT_anim_command items[1024];
	int length;
} CAT_draw_queue;

void CAT_anim_queue_init(CAT_draw_queue* queue)
{
	queue->length = 0;
}

void CAT_anim_queue_add(CAT_anim_queue* queue, CAT_anim_command cmd)
{
	if(queue->length == 1024)
	{
		return;
	}

	int insert_idx = queue->length;
	for(int i = 0; i < queue->length; i++)
	{
		CAT_anim_command other = queue->items[i];
		if(cmd.layer <= other.layer && cmd.y < other.y)
		{
			insert_idx = i;
			break;
		}
	}

	for(int i = queue->length; i > insert_idx; i--)
	{
		queue->items[i] = queue->items[i-1];
	}
	queue->items[insert_idx] = cmd;
	queue->length += 1;
}

void CAT_anim_queue_tick(CAT_anim_queue* queue)
{
	for(int i = 0; i < queue->length; i++)
	{
		CAT_anim_command cmd = queue->items[i];
		CAT_anim* anim = cmd.anim;
		CAT_anim_tick(anim);
	}
}

void CAT_anim_queue_draw(CAT_anim_queue* queue, CAT_atlas* atlas, CAT_texture* frame)
{
	for(int i = 0; i < queue->length; i++)
	{
		CAT_anim_command cmd = queue->items[i];
		CAT_anim* anim = cmd.anim;
		CAT_draw_sprite
		(
			frame, cmd.x, cmd.y,
			atlas, anim->frames[anim->idx],
			cmd.mode
		);
	}
	queue->length = 0;
}

typedef struct CAT_gui
{
	int tiles[9];
	int glyphs[91];

	CAT_ivec2 start;
	CAT_ivec2 shape;
	int margin;

	CAT_ivec2 cursor;
	CAT_ivec2 cursor_last;
} CAT_gui;

void CAT_gui_row(CAT_gui* gui, CAT_atlas* atlas, CAT_texture* frame, int stage)
{
	CAT_draw_sprite(frame, gui->cursor.x, gui->cursor.y, atlas, gui->tiles[stage*3+0], CAT_DRAW_MODE_DEFAULT);
	gui->cursor.x += 16;
	for(int col = 1; col < gui->shape.x-1; col++)
	{
		CAT_draw_sprite(frame, gui->cursor.x, gui->cursor.y, atlas, gui->tiles[stage*3+1], CAT_DRAW_MODE_DEFAULT);
		gui->cursor.x += 16;
	}
	CAT_draw_sprite(frame, gui->cursor.x, gui->cursor.y, atlas, gui->tiles[stage*3+2], CAT_DRAW_MODE_DEFAULT);
	gui->cursor.x += 16;

	gui->cursor.y += 16;
	gui->cursor.x = gui->start.x;
}

void CAT_gui_panel(CAT_gui* gui, CAT_atlas* atlas, CAT_texture* frame, CAT_ivec2 start, CAT_ivec2 shape)
{
	gui->start = start;
	gui->shape = shape;
	gui->cursor = start;

	CAT_gui_row(gui, atlas, frame, 0);
	for(int row = 1; row < gui->shape.y-1; row++)
	{
		CAT_gui_row(gui, atlas, frame, 1);
	}
	CAT_gui_row(gui, atlas, frame, 2);

	gui->cursor = gui->start;
	gui->cursor.y += gui->margin;
	gui->cursor.x += gui->margin;
}

void CAT_gui_line_break(CAT_gui* gui)
{
	gui->cursor_last = gui->cursor;
	gui->cursor.y += 16;
	gui->cursor.x = gui->start.x + gui->margin;
}

void CAT_gui_same_line(CAT_gui* gui)
{
	gui->cursor = gui->cursor_last;
	gui->cursor.x += gui->margin;
}

void CAT_gui_text(CAT_gui* gui, CAT_atlas* atlas, CAT_texture* frame, char* text)
{
	for(char* c = text; *c != '\0'; c++)
	{
		if(*c == '\n')
		{
			CAT_gui_line_break(gui);
			continue;
		}

		CAT_draw_sprite(frame, gui->cursor.x, gui->cursor.y, atlas, gui->glyphs[(*c)-' '], CAT_DRAW_MODE_DEFAULT);
		gui->cursor.x += gui->margin;
	}
	
	CAT_gui_line_break(gui);
}

#endif
