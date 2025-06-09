#include "cat_arcade.h"

#include "cat_render.h"
#include "cat_input.h"
#include "cat_math.h"
#include "cat_gui.h"
#include "cat_room.h"

#define CAT_FOURSQUARES_TILE_SIZE 16
#define CAT_FOURSQUARES_GRID_WIDTH (CAT_LCD_SCREEN_W / CAT_FOURSQUARES_TILE_SIZE)
#define CAT_FOURSQUARES_GRID_HEIGHT (CAT_LCD_SCREEN_H / CAT_FOURSQUARES_TILE_SIZE)
#define CAT_FOURSQUARES_GRID_SIZE (CAT_FOURSQUARES_GRID_WIDTH * CAT_FOURSQUARES_GRID_HEIGHT)
#define CAT_FOURSQUARES_MAX_PIECE_COUNT (CAT_FOURSQUARES_GRID_SIZE / 4)

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

typedef enum
{
	CAT_FOURSQUARES_CW,
	CAT_FOURSQUARES_CCW
} CAT_foursquares_rotation;

typedef struct
{
	bool full;
	uint16_t colour;
} CAT_foursquares_cell;

static uint16_t piece_colours[7] =
{
	0x0617,
	0x2ad8,
	0xeca0,
	0xdec1,
	0x6de3,
	0xaa7a,
	0xd8e3,
};

static CAT_ivec2 jlstz_kick_offset_bases[5] =
{
	{0, 0},
	{-1, 0},
	{-1, 1},
	{0, -2},
	{-1, -2}
};

static CAT_ivec2 jlstz_kick_offset_signs[5] =
{
	{1, 1},
	{-1, -1},
	{1, 1},
	{1, -1},
	{-1, 1}
};

static CAT_ivec2 i_kick_offsets[4][5] =
{
	{
		{0, 0},
		{-2, 0},
		{1, 0},
		{-2, -1},
		{1, 2}
	},
	{
		{0, 0},
		{-1, 0},
		{2, 0},
		{-1, 2},
		{2, 1}
	},
	{
		{0, 0},
		{2, 0},
		{-1, 0},
		{2, 1},
		{-1, 2}
	},
	{
		{0, 0},
		{1, 0},
		{-2, 0},
		{1, -2},
		{-2, 1}
	}
};

static CAT_foursquares_cell cells[CAT_FOURSQUARES_GRID_HEIGHT][CAT_FOURSQUARES_GRID_WIDTH];

static CAT_foursquares_type seven_bag[7] =
{
	CAT_FOURSQUARES_I,
	CAT_FOURSQUARES_J,
	CAT_FOURSQUARES_L,
	CAT_FOURSQUARES_O,
	CAT_FOURSQUARES_S,
	CAT_FOURSQUARES_T,
	CAT_FOURSQUARES_Z
};
static int bag_idx = 0;

static CAT_foursquares_type piece_type = CAT_FOURSQUARES_I;
static CAT_ivec2 piece_position = {0, 0};
static CAT_foursquares_orientation piece_orientation = CAT_FOURSQUARES_NORTH;
static uint8_t collider[4][4];
static uint8_t collider_w = 4;
static uint8_t collider_h = 4;

static CAT_ivec2 piece_position_buffer = {0, 0};
static CAT_foursquares_orientation piece_orientation_buffer = CAT_FOURSQUARES_NORTH;
static uint8_t collider_buffer[4][4];

static int score = 0;

void reset_seven_bag()
{
	for(int i = 0; i < 7; i++)
	{
		int j = CAT_rand_int(0, 6);
		int temp = seven_bag[i];
		seven_bag[i] = seven_bag[j];
		seven_bag[j] = temp;
	}

	bag_idx = 0;
}

CAT_foursquares_type access_seven_bag()
{
	if(bag_idx >= 7)
		reset_seven_bag();
	return seven_bag[bag_idx++];
}

void configure_collider
(
	uint8_t m00, uint8_t m01, uint8_t m02, uint8_t m03,
	uint8_t m10, uint8_t m11, uint8_t m12, uint8_t m13,
	uint8_t m20, uint8_t m21, uint8_t m22, uint8_t m23,
	uint8_t m30, uint8_t m31, uint8_t m32, uint8_t m33,
	uint8_t w, uint8_t h
)
{
	collider[0][0] = m00; collider[0][1] = m01; collider[0][2] = m02; collider[0][3] = m03;
	collider[1][0] = m10; collider[1][1] = m11; collider[1][2] = m12; collider[1][3] = m13;
	collider[2][0] = m20; collider[2][1] = m21; collider[2][2] = m22; collider[2][3] = m23;
	collider[3][0] = m30; collider[3][1] = m31; collider[3][2] = m32; collider[3][3] = m33;
	collider_w = w; collider_h = h;
}

void spawn_piece(CAT_foursquares_type type, CAT_ivec2 position)
{
	piece_type = type;
	piece_orientation = CAT_FOURSQUARES_NORTH;
	piece_position = position;

	switch (type)
	{
		case CAT_FOURSQUARES_I:
			configure_collider
			(
				0, 0, 0, 0,
				1, 1, 1, 1,
				0, 0, 0, 0,
				0, 0, 0, 0,
				4, 4
			);
			piece_position.y -= 1;
		break;

		case CAT_FOURSQUARES_J:
			configure_collider
			(
				1, 0, 0, 0,
				1, 1, 1, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,
				3, 3
			);
		break;
		
		case CAT_FOURSQUARES_L:
			configure_collider
			(
				0, 0, 1, 0,
				1, 1, 1, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,
				3, 3
			);
		break;

		case CAT_FOURSQUARES_O:
			configure_collider
			(
				0, 1, 1, 0,
				0, 1, 1, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,
				4, 3
			);
		break;

		case CAT_FOURSQUARES_S:
			configure_collider
			(
				0, 1, 1, 0,
				1, 1, 0, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,
				3, 3
			);
		break;

		case CAT_FOURSQUARES_T:
			configure_collider
			(
				0, 1, 0, 0,
				1, 1, 1, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,
				3, 3
			);
		break;

		case CAT_FOURSQUARES_Z:
			configure_collider
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

bool is_blocked_out()
{
	for(int dy = 0; dy < collider_h; dy++)
	{
		int y = piece_position.y + dy;
		if(y < 0 || y >= CAT_FOURSQUARES_GRID_HEIGHT)
			continue;

		for(int dx = 0; dx < collider_w; dx++)
		{
			int x = piece_position.x + dx;
			if(x < 0 || x >= CAT_FOURSQUARES_GRID_WIDTH)
				continue;

			if(collider[dy][dx] && cells[y][x].full)
				return true;
		}
	}

	return false;
}

void reset_buffers()
{
	piece_position_buffer = piece_position;
	piece_orientation_buffer = piece_orientation;
	for(int y = 0; y < collider_h; y++)
	{
		for(int x = 0; x < collider_w; x++)
		{
			collider_buffer[y][x] = collider[y][x];
		}
	}
}

void move_piece(int dx, int dy)
{
	piece_position_buffer.x += dx;
	piece_position_buffer.y += dy;
}

void rotate_piece(CAT_foursquares_rotation rotation)
{	
	int rotation_offset = rotation == CAT_FOURSQUARES_CW ? 1 : -1;
	piece_orientation_buffer = (piece_orientation + rotation_offset + 4) % 4;

	if(piece_type == CAT_FOURSQUARES_O)
		return;

	for(int y = 0; y < collider_h; y++)
	{
		for(int x = 0; x < collider_w; x++)
		{
			if(rotation == CAT_FOURSQUARES_CW)
				collider_buffer[x][collider_h - y - 1] = collider[y][x];
			else
				collider_buffer[y][x] = collider[x][collider_h - y - 1];
		}
	}
}

bool are_buffers_valid()
{
	for(int dy = 0; dy < collider_h; dy++)
	{
		for(int dx = 0; dx < collider_w; dx++)
		{
			if(collider_buffer[dy][dx])
			{
				int x = piece_position_buffer.x + dx;
				int y = piece_position_buffer.y + dy;

				if(y >= CAT_FOURSQUARES_GRID_HEIGHT)
					return false;
				
				if(x < 0)
					return false;
				if(x >= CAT_FOURSQUARES_GRID_WIDTH)
					return false;

				if(cells[y][x].full)
					return false;
			}
		}
	}

	return true;
}

void commit_buffers()
{
	piece_position = piece_position_buffer;
	piece_orientation = piece_orientation_buffer;
	for(int y = 0; y < collider_h; y++)
	{
		for(int x = 0; x < collider_w; x++)
		{
			collider[y][x] = collider_buffer[y][x];
		}
	}
}

bool enumerate_kicks(CAT_foursquares_rotation rotation)
{
	int kick_case = rotation == CAT_FOURSQUARES_CW ? piece_orientation : piece_orientation_buffer;
	int kick_dir = rotation == CAT_FOURSQUARES_CW ? 1 : -1;

	for(int i = 0; i < 5; i++)
	{
		switch (piece_type)
		{
			case CAT_FOURSQUARES_J:
			case CAT_FOURSQUARES_L:
			case CAT_FOURSQUARES_S:
			case CAT_FOURSQUARES_T:
			case CAT_FOURSQUARES_Z:
			{
				CAT_ivec2 base = jlstz_kick_offset_bases[i];
				CAT_ivec2 signs = jlstz_kick_offset_signs[kick_case];
				CAT_ivec2 offset = {base.x * signs.x, base.y * signs.y};
				offset = CAT_ivec2_mul(offset, kick_dir);
				piece_position_buffer = CAT_ivec2_add(piece_position, offset);
				if(are_buffers_valid())
					return true;
				break;
			}
			
			case CAT_FOURSQUARES_I:
			{
				CAT_ivec2 offset = i_kick_offsets[kick_case][i];
				offset = CAT_ivec2_mul(offset, kick_dir);
				piece_position_buffer = CAT_ivec2_add(piece_position, offset);
				if(are_buffers_valid())
					return true;
				break;
			}
		
			default:
			{
				return true;
			}
		}
	}

	return false;
}

bool is_piece_settled()
{
	for(int dy = 0; dy < collider_h; dy++)
	{
		for(int dx = 0; dx < collider_w; dx++)
		{
			if(collider[dy][dx])
			{
				int x = piece_position.x + dx;
				int y = piece_position.y + dy;

				if(y < CAT_FOURSQUARES_GRID_HEIGHT-1)
				{
					if(cells[y+1][x].full)
						return true;
				}
				else
					return true;
			}
		}
	}

	return false;
}

void lock_piece()
{
	for(int dy = 0; dy < collider_h; dy++)
	{
		for(int dx = 0; dx < collider_w; dx++)
		{
			if(collider[dy][dx])
			{
				int x = piece_position.x + dx;
				int y = piece_position.y + dy;

				cells[y][x].full = true;
				cells[y][x].colour = piece_colours[piece_type];
			}
		}
	}
}

void perform_reckoning()
{
	int clear_start = CAT_FOURSQUARES_GRID_HEIGHT;
	int clear_end = -1;

	for(int y = 0; y < CAT_FOURSQUARES_GRID_HEIGHT; y++)
	{
		bool full_row = true;

		for(int x = 0; x < CAT_FOURSQUARES_GRID_WIDTH; x++)
		{
			if(!cells[y][x].full)
			{
				full_row = false;
				break;
			}
		}

		if(full_row)
		{
			clear_start = min(clear_start, y);
			clear_end = max(clear_end, y);
		}
	}

	if(clear_start > clear_end)
		return;

	for(int y = clear_start; y <= clear_end; y++)
	{
		for(int x = 0; x < CAT_FOURSQUARES_GRID_WIDTH; x++)
		{
			cells[y][x].full = false;
		}
	}

	int rows_shifted = 0;
	for(int y = clear_start-1; y >= 0; y--)
	{
		for(int x = 0; x < CAT_FOURSQUARES_GRID_WIDTH; x++)
		{
			cells[clear_end-rows_shifted][x] = cells[y][x];
			cells[y][x].full = false;
		}
		rows_shifted += 1;
	}

	int rows_cleared = clear_end - clear_start + 1;
	switch(rows_cleared)
	{
		case 4:
			score += 1200;
		break;
		case 3:
			score += 300;
		break;
		case 2:
			score += 100;
		break;
		default:
			score += 40;
		break;
	}
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

void render_game_over()
{
	CAT_frameberry(CAT_BLACK);

	int cursor_y = 12;
	CAT_set_text_scale(2);
	CAT_set_text_colour(CAT_WHITE);
	CAT_draw_text(12, cursor_y, "Game over...");
	cursor_y += 52;

	CAT_set_text_colour(CAT_WHITE);
	CAT_draw_text(12, cursor_y, "Today's score:");
	cursor_y += 16;
	CAT_set_text_scale(2);
	CAT_set_text_colour(CAT_WHITE);
	CAT_draw_textf(12, cursor_y, "%d POINTS!", score);
	cursor_y += 52;

	CAT_set_text_colour(CAT_RED);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	CAT_set_text_mask(12, -1, CAT_LCD_SCREEN_W-12, -1);
	CAT_draw_text(12, cursor_y, "... for the rectilinear belongs only to Geometry and not to Nature and Life.");
}

void MS_game_over(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			CAT_set_render_callback(render_game_over);
		break;

		case CAT_MACHINE_SIGNAL_TICK:
			if(CAT_input_pressed(CAT_BUTTON_A) || CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_room);
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}

void CAT_MS_foursquares(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			CAT_set_render_callback(CAT_render_foursquares);

			score = 0;

			for(int y = 0; y < CAT_FOURSQUARES_GRID_HEIGHT; y++)
			{
				for(int x = 0; x < CAT_FOURSQUARES_GRID_WIDTH; x++)
				{
					cells[y][x].full = false;
				}
			}

			reset_seven_bag();
			spawn_piece
			(
				access_seven_bag(),
				(CAT_ivec2) {CAT_rand_int(0, CAT_FOURSQUARES_GRID_WIDTH-collider_w), 0}
			);
		break;

		case CAT_MACHINE_SIGNAL_TICK:
		{
			static bool quit = false;
			if(CAT_input_pressed(CAT_BUTTON_START) || CAT_input_held(CAT_BUTTON_B, 0.5f))
				CAT_gui_open_popup("Quit Foursquares?\n\nProgress will not\nbe saved!\n", &quit);
			if(quit)
			{
				quit = false;
				CAT_machine_back();
			}
			if(CAT_gui_popup_is_open())
				break;

			reset_buffers();

			if(CAT_input_pressed(CAT_BUTTON_A))
			{	
				rotate_piece(CAT_FOURSQUARES_CW);
				if(are_buffers_valid() || enumerate_kicks(CAT_FOURSQUARES_CW))
					commit_buffers();
			}
			if(CAT_input_pressed(CAT_BUTTON_B))
			{	
				rotate_piece(CAT_FOURSQUARES_CCW);
				if(are_buffers_valid() || enumerate_kicks(CAT_FOURSQUARES_CCW))
					commit_buffers();
				else
					reset_buffers();
			}

			if(CAT_input_pulse(CAT_BUTTON_LEFT))
				move_piece(-1, 0);
			if(CAT_input_pulse(CAT_BUTTON_RIGHT))
				move_piece(1, 0);
			if(are_buffers_valid())
				commit_buffers();
			else
				reset_buffers();

			if(CAT_input_pulse(CAT_BUTTON_DOWN))
				move_piece(0, 1);
			if(are_buffers_valid())
				commit_buffers();
			else
				reset_buffers();

			if(CAT_input_pressed(CAT_BUTTON_UP))
			{
				while(are_buffers_valid())
				{
					commit_buffers();
					move_piece(0, 1);
				}
				reset_buffers();
			}

			if(should_trigger_step())
			{
				move_piece(0, 1);
				if(are_buffers_valid())
					commit_buffers();
				else
				{
					if(is_piece_settled())
					{
						lock_piece();
						perform_reckoning();
						spawn_piece
						(
							access_seven_bag(),
							(CAT_ivec2) {CAT_rand_int(0, CAT_FOURSQUARES_GRID_WIDTH-collider_w), 0}
						);
						if(is_blocked_out())
							CAT_machine_transition(MS_game_over);
					}
					reset_buffers();
				}		
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
			CAT_foursquares_cell* cell = &cells[y][x];
			if(cell->full)
			{
				CAT_fillberry
				(
					x * CAT_FOURSQUARES_TILE_SIZE, y * CAT_FOURSQUARES_TILE_SIZE,
					CAT_FOURSQUARES_TILE_SIZE, CAT_FOURSQUARES_TILE_SIZE,
					cell->colour
				);
			}
		}
	}

	for(int i = 0; i < collider_h; i++)
	{
		int y_p = piece_position.y + i;
		for(int j = 0; j < collider_w; j++)
		{
			int x_p = piece_position.x + j;

			if(collider[i][j])
			{
				CAT_fillberry
				(
					x_p * CAT_FOURSQUARES_TILE_SIZE, y_p * CAT_FOURSQUARES_TILE_SIZE,
					CAT_FOURSQUARES_TILE_SIZE, CAT_FOURSQUARES_TILE_SIZE,
					piece_colours[piece_type]
				);
			}
		}
	}
}