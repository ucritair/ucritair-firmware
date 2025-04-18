#include "cat_arcade.h"

#include "cat_render.h"
#include "cat_input.h"
#include "cat_math.h"
#include "cat_gui.h"

#define CAT_FOURSQUARES_TILE_SIZE 24
#define CAT_FOURSQUARES_GRID_WIDTH (CAT_LCD_SCREEN_W / CAT_FOURSQUARES_TILE_SIZE)
#define CAT_FOURSQUARES_GRID_HEIGHT (CAT_LCD_SCREEN_H / CAT_FOURSQUARES_TILE_SIZE)
#define CAT_FOURSQUARES_GRID_SIZE (CAT_FOURSQUARES_GRID_WIDTH * CAT_FOURSQUARES_GRID_HEIGHT)
#define CAT_FOURSQUARES_MAX_PIECE_COUNT (CAT_FOURSQUARES_GRID_SIZE / 4)

typedef enum
{
	CAT_FOURSQUARES_EMPTY,
	CAT_FOURSQUARES_FULL
} CAT_foursquares_state;

typedef enum
{
 	CAT_FOURSQUARES_I,
	CAT_FOURSQUARES_J,
	CAT_FOURSQUARES_L,
	CAT_FOURSQUARES_O,
	CAT_FOURSQUARES_S,
	CAT_FOURSQUARES_T,
	CAT_FOURSQUARES_Z
} CAT_foursquares_type;

typedef enum
{
	CAT_FOURSQUARES_NORTH,
	CAT_FOURSQUARES_EAST,
	CAT_FOURSQUARES_SOUTH,
	CAT_FOURSQUARES_WEST
} CAT_foursquares_orientation;

typedef struct
{
	CAT_foursquares_type type;
	CAT_foursquares_orientation orientation;
	CAT_ivec2 position;
} CAT_foursquares_piece;

typedef struct
{
	CAT_foursquares_state state;
	uint16_t colour;
} CAT_foursquares_cell;

static uint16_t piece_colours[7] =
{
	0xF000,
	0x0F00,
	0x00F0,
	0x000F,
	0xF00F,
	0xFF00,
	0xF0F0,
};

static CAT_foursquares_cell cells[CAT_FOURSQUARES_GRID_SIZE];
static CAT_foursquares_piece piece;

static uint8_t collision_matrix[4][4];
static uint8_t collision_w = 4;
static uint8_t collision_h = 4;
static uint8_t rotation_buffer_matrix[4][4];

void dump_collision_matrix()
{
	for(int y = 0; y < collision_h; y++)
	{
		for(int x = 0; x < collision_w; x++)
		{
			CAT_printf("%d ", collision_matrix[y][x]);
		}
		CAT_printf("\n");
	}
	CAT_printf("%d %d\n", collision_w, collision_h);
}

void configure_collision_matrix
(
	uint8_t m00, uint8_t m01, uint8_t m02, uint8_t m03,
	uint8_t m10, uint8_t m11, uint8_t m12, uint8_t m13,
	uint8_t m20, uint8_t m21, uint8_t m22, uint8_t m23,
	uint8_t m30, uint8_t m31, uint8_t m32, uint8_t m33,
	uint8_t w, uint8_t h
)
{
	collision_matrix[0][0] = m00; collision_matrix[0][1] = m01; collision_matrix[0][2] = m02; collision_matrix[0][3] = m03;
	collision_matrix[1][0] = m10; collision_matrix[1][1] = m11; collision_matrix[1][2] = m12; collision_matrix[1][3] = m13;
	collision_matrix[2][0] = m20; collision_matrix[2][1] = m21; collision_matrix[2][2] = m22; collision_matrix[2][3] = m23;
	collision_matrix[3][0] = m30; collision_matrix[3][1] = m31; collision_matrix[3][2] = m32; collision_matrix[3][3] = m33;
	collision_w = w; collision_h = h;
}

void spawn_piece(CAT_foursquares_type type, CAT_ivec2 position)
{
	piece = (CAT_foursquares_piece)
	{
		.type = type,
		.orientation = CAT_FOURSQUARES_NORTH,
		.position = position
	};

	switch (type)
	{
		case CAT_FOURSQUARES_I:
			configure_collision_matrix
			(
				0, 0, 0, 0,
				1, 1, 1, 1,
				0, 0, 0, 0,
				0, 0, 0, 0,
				4, 4
			);
		break;

		case CAT_FOURSQUARES_J:
			configure_collision_matrix
			(
				0, 0, 1, 0,
				1, 1, 1, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,
				3, 3
			);
		break;
		
		case CAT_FOURSQUARES_L:
			configure_collision_matrix
			(
				1, 0, 0, 0,
				1, 1, 1, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,
				3, 3
			);
		break;

		case CAT_FOURSQUARES_O:
			configure_collision_matrix
			(
				0, 1, 1, 0,
				0, 1, 1, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,
				4, 3
			);
		break;

		case CAT_FOURSQUARES_S:
			configure_collision_matrix
			(
				0, 1, 1, 0,
				1, 1, 0, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,
				3, 3
			);
		break;

		case CAT_FOURSQUARES_T:
			configure_collision_matrix
			(
				0, 1, 0, 0,
				1, 1, 1, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,
				3, 3
			);
		break;

		case CAT_FOURSQUARES_Z:
			configure_collision_matrix
			(
				1, 1, 0, 0,
				0, 1, 1, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,
				3, 3
			);
		break;
	}
}

void rotate_piece()
{	
	for(int y = 0; y < 4; y++)
	{
		for(int x = 0; x < 4; x++)
		{
			rotation_buffer_matrix[x][4 - y - 1] = collision_matrix[y][x];
		}
	}

	for(int y = 0; y < 4; y++)
	{
		for(int x = 0; x < 4; x++)
		{
			collision_matrix[y][x] = rotation_buffer_matrix[y][x];
		}
	}
}

void movement_step()
{
	piece.position.y += 1;
}

static int frame_counter = 0;
bool should_trigger_step()
{
	if(frame_counter == 4)
	{
		frame_counter = 0;
		return true;
	}
	frame_counter += 1;
	return false;
}

void CAT_MS_foursquares(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			CAT_set_render_callback(CAT_render_foursquares);

			for(int i = 0; i < CAT_FOURSQUARES_GRID_SIZE; i++)
			{
				cells[i] = (CAT_foursquares_cell)
				{
					.state = CAT_FOURSQUARES_EMPTY
				};
			}

			spawn_piece(CAT_FOURSQUARES_I, (CAT_ivec2) {0, 0});
		break;

		case CAT_MACHINE_SIGNAL_TICK:
		{
			static bool quit = false;
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_gui_open_popup("Quit Foursquares?\n\nProgress will not\nbe saved!\n", &quit);
			if(quit)
			{
				quit = false;
				CAT_machine_back();
			}

			if(CAT_input_pressed(CAT_BUTTON_A))
			{
				rotate_piece();
				dump_collision_matrix();
			}

			if(should_trigger_step())
			{
				movement_step();
			}
		break;
		}

		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}

void CAT_render_foursquares()
{
	CAT_frameberry(0);

	for(int y = 0; y < CAT_FOURSQUARES_GRID_HEIGHT; y++)
	{
		for(int x = 0; x < CAT_FOURSQUARES_GRID_WIDTH; x++)
		{
			CAT_strokeberry
			(
				x * CAT_FOURSQUARES_TILE_SIZE, y * CAT_FOURSQUARES_TILE_SIZE,
				CAT_FOURSQUARES_TILE_SIZE, CAT_FOURSQUARES_TILE_SIZE,
				0xFFFF
			);

			CAT_foursquares_cell* cell = &cells[y * CAT_FOURSQUARES_GRID_WIDTH + x];
			if(cell->state != CAT_FOURSQUARES_EMPTY)
			{
				CAT_fillberry
				(
					x * CAT_FOURSQUARES_TILE_SIZE, y * CAT_FOURSQUARES_TILE_SIZE,
					CAT_FOURSQUARES_TILE_SIZE, CAT_FOURSQUARES_TILE_SIZE,
					cell->colour
				);
			}

			for(int i = 0; i < collision_h; i++)
			{
				int y_p = piece.position.y + i;
				for(int j = 0; j < collision_w; j++)
				{
					int x_p = piece.position.x + j;
					if(collision_matrix[i][j])
					{
						CAT_fillberry
						(
							x_p * CAT_FOURSQUARES_TILE_SIZE, y_p * CAT_FOURSQUARES_TILE_SIZE,
							CAT_FOURSQUARES_TILE_SIZE, CAT_FOURSQUARES_TILE_SIZE,
							piece_colours[piece.type]
						);
					}
				}
			}
		}
	}
}