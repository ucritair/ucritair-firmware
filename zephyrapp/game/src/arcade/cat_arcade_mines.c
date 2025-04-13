#include "cat_arcade.h"

#include "cat_input.h"
#include "cat_render.h"
#include "cat_room.h"
#include "cat_bag.h"
#include "cat_gui.h"
#include "sprite_assets.h"
#include "sound_assets.h"

#define GRID_WIDTH 15
#define GRID_HEIGHT 20
#define GRID_SIZE (GRID_WIDTH * GRID_HEIGHT)
#define MINE_COUNT (GRID_SIZE / 7)

typedef struct grid_cell
{
	int x;
	int y;
	int idx;

	bool mine;
	bool seen;
	bool flagged;
	bool visited;

	bool coin;
	float coin_timer;

	int adjacent;
} grid_cell;
static grid_cell grid[GRID_SIZE];

static grid_cell* get_cell(int x, int y)
{
	if(x < 0 || x >= GRID_WIDTH)
		return NULL;
	if(y < 0 || y >= GRID_HEIGHT)
		return NULL;
	return &grid[y * GRID_WIDTH + x];
}

static bool is_cell_clean(int x, int y)
{
	grid_cell* cell = get_cell(x, y);
	if(cell == NULL)
		return false;
	return !cell->visited && !cell->seen;
}
static int idx_queue[GRID_SIZE];
static int idx_queue_length = 0;

static void idx_enqueue(int idx)
{
	if(idx_queue_length >= GRID_SIZE)
		return;

	idx_queue[idx_queue_length] = idx;
	idx_queue_length += 1;
}

static int idx_dequeue()
{
	if(idx_queue_length <= 0)
		return -1;

	int idx = idx_queue[0];
	idx_queue_length -= 1;
	for(int i = 0; i < idx_queue_length; i++)
	{
		idx_queue[i] = idx_queue[i+1];
	}
	return idx;
}

static enum {PLAY, LOSE, WIN} state = PLAY;
static CAT_ivec2 cursor = {0, 0};
static bool first_click = true;
static int clicks = 0;
static int unseen = 0;

static int reveal_timer_id = -1;
static bool reveal_complete = false;

void toggle_mine(int x, int y, bool value)
{
	grid_cell* cell = get_cell(x, y);
	int da = (cell->mine == value) ? 0 : (value ? 1 : -1);
	cell->mine = value;
	for(int dy = -1; dy <= 1; dy++)
	{
		for(int dx = -1; dx <= 1; dx++)
		{
			grid_cell* neighbour = get_cell(x+dx, y+dy);
			if(neighbour != NULL)
			{
				neighbour->adjacent += da;
			}	
		}
	}
}

void init_grid()
{
	for(int y = 0; y < GRID_HEIGHT; y++)
	{
		for(int x = 0; x < GRID_WIDTH; x++)
		{
			int idx = y * GRID_WIDTH + x;
			grid[idx] = (grid_cell)
			{
				.x = x,
				.y = y,
				.idx = idx,

				.mine = false,
				.seen = false,
				.flagged = false,
				.visited = false,

				.coin = false,
				.coin_timer = 0.0f,

				.adjacent = 0
			};
		}
	}

	idx_queue_length = 0;
	for(int i = 0; i < GRID_SIZE; i++)
		idx_enqueue(i);
	for(int i = 0; i < idx_queue_length; i++)
	{
		int temp = idx_queue[i];
		int rand = CAT_rand_int(0, idx_queue_length-1);
		idx_queue[i] = idx_queue[rand];
		idx_queue[rand] = temp;
	}

	for(int i = 0; i < MINE_COUNT; i++)
	{
		idx_queue_length -= 1;
		int idx = idx_queue[idx_queue_length];
		int x = grid[idx].x;
		int y = grid[idx].y;
		toggle_mine(x, y, true);
	}

	unseen = GRID_SIZE;
}

void flood_reveal(int x, int y)
{
	idx_queue_length = 0;
	idx_enqueue(y * GRID_WIDTH + x);
	while(idx_queue_length > 0)
	{		
		grid_cell* c = &grid[idx_dequeue()];
		c->seen = true;
		unseen -= 1;
		if(c->adjacent > 0)
			continue;

		for(int dy = -1; dy <= 1; dy++)
		{
			for(int dx = -1; dx <= 1; dx++)
			{
				if(dx == 0 && dy == 0)
					continue;
				int nidx = (c->y+dy) * CAT_GRID_WIDTH + (c->x+dx);
				if(is_cell_clean(c->x+dx, c->y+dy))
				{
					grid[nidx].visited = true;
					idx_enqueue(nidx);
				}
			}
		}
	}
}

void shuffle_about(int x, int y)
{
	idx_queue_length = 0;
	for(int i = 0; i < GRID_SIZE; i++)
	{
		if(!grid[i].mine)
			idx_enqueue(i);
	}
	for(int i = 0; i < idx_queue_length; i++)
	{
		int temp = idx_queue[i];
		int rand = CAT_rand_int(0, idx_queue_length-1);
		idx_queue[i] = idx_queue[rand];
		idx_queue[rand] = temp;
	}

	for(int dy = -1; dy <= 1; dy++)
	{
		for(int dx = -1; dx <= 1; dx++)
		{
			grid_cell* cand = get_cell(x+dx, y+dy);
			if(cand != NULL && cand->mine)
			{
				toggle_mine(x+dx, y+dy, false);

				idx_queue_length -= 1;
				int mov_idx = idx_queue[idx_queue_length];
				int mov_x = grid[mov_idx].x;
				int mov_y = grid[mov_idx].y;
				toggle_mine(mov_x, mov_y, true);
			}
		}
	}
}

void CAT_MS_mines(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{		
			CAT_set_render_callback(CAT_render_mines);

			init_grid();

			state = PLAY;
			first_click = true;
			clicks = 0;

			if(reveal_timer_id == -1)
				reveal_timer_id = CAT_timer_init(0.05f);
			reveal_complete = false;
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{	
			if(state == PLAY)
			{
				static bool quit = false;
				if(CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
						CAT_gui_open_popup("Quit Sweep?\n\nProgress will not be saved!\n\n", &quit);
				if(quit)
				{
					quit = false;
					CAT_machine_transition(CAT_MS_room);
				}		
				if(CAT_gui_popup_is_open())
					break;

				if(CAT_input_pulse(CAT_BUTTON_UP))
					cursor.y -= 1;
				if(CAT_input_pulse(CAT_BUTTON_RIGHT))
					cursor.x += 1;
				if(CAT_input_pulse(CAT_BUTTON_DOWN))
					cursor.y += 1;
				if(CAT_input_pulse(CAT_BUTTON_LEFT))
					cursor.x -= 1;
				cursor.x = clamp(cursor.x, 0, GRID_WIDTH-1);
				cursor.y = clamp(cursor.y, 0, GRID_HEIGHT-1);
				grid_cell* cell = get_cell(cursor.x, cursor.y);

				if(CAT_input_pressed(CAT_BUTTON_A) && !cell->flagged)
				{
					if(first_click)
					{
						shuffle_about(cursor.x, cursor.y);
						first_click = false;
					}
					
					if(!cell->seen)
					{
						cell->seen = true;

						if(cell->mine)
						{
							state = LOSE;
							CAT_play_sound(&fail_sound);
							break;
						}

						clicks += 1;
						if(clicks == 8)
						{
							coins += 1;
							cell->coin = true;
							CAT_play_sound(&coin_sound);
							clicks = 0;
						}

						flood_reveal(cursor.x, cursor.y);

						if(unseen == MINE_COUNT)
						{
							state = WIN;
						}
					}				
				}
				if(CAT_input_pressed(CAT_BUTTON_SELECT))
					cell->flagged = !cell->flagged;
				
				for(int i = 0; i < GRID_SIZE; i++)
				{
					if(grid[i].coin)
						grid[i].coin_timer += CAT_get_delta_time_s();
				}
			}
			else
			{
				if(!reveal_complete)
				{
					if(CAT_timer_tick(reveal_timer_id))
					{
						CAT_timer_reset(reveal_timer_id);
						reveal_complete = true;
						for(int i = 0; i < GRID_SIZE; i++)
						{
							if(grid[i].mine && !grid[i].seen)
							{
								grid[i].seen = true;
								reveal_complete = false;
								CAT_play_sound(&thud_sound);
								break;
							}	
						}
					}
					
					if(CAT_input_pressed(CAT_BUTTON_A))
						reveal_complete = true;
				}
				else
				{
					if(CAT_input_pressed(CAT_BUTTON_A))
					{
						if(state == WIN)
						{
							CAT_item_list_add(&bag, prop_mine_item, 1);
						}
						CAT_machine_back();
					}	
				}
				break;
			}

			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			break;
		}
	}
}

void CAT_render_mines()
{
	if(!reveal_complete)
	{
		draw_mode = CAT_DRAW_MODE_DEFAULT;
		for(int y = 0; y < GRID_HEIGHT; y++)
		{
			for(int x = 0; x < GRID_WIDTH; x++)
			{
				grid_cell* cell = get_cell(x, y);
				int xs = x * CAT_TILE_SIZE;
				int ys = y * CAT_TILE_SIZE;

				if(cell->seen)
				{
					if(cell->mine)
					{
						CAT_draw_sprite(&mines_sprite, 10, xs, ys);
					}
					else
					{
						CAT_draw_sprite(&mines_sprite, 1, xs, ys);
						if(cell->adjacent > 0)
							CAT_draw_sprite(&mines_sprite, 1 + cell->adjacent, xs, ys);
						if(cell->coin && cell->coin_timer <= 1.0f)
							CAT_draw_sprite(&coin_world_sprite, 0, xs, ys);
					}
				}
				else
				{
					CAT_draw_sprite(&mines_sprite, 0, xs, ys);
					if(cell->flagged)
						CAT_draw_sprite(&mines_sprite, 12, xs, ys);
				}
			}
		}
		if(state == PLAY)
			CAT_draw_sprite(&mines_sprite, 11, cursor.x*CAT_TILE_SIZE, cursor.y*CAT_TILE_SIZE);
	}
	else
	{
		CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 20});

		draw_mode = CAT_DRAW_MODE_DEFAULT;
		for(int x = 0; x < GRID_WIDTH; x++)
		{
			CAT_draw_sprite(&mines_sprite, 10, x * CAT_TILE_SIZE, 0);
			CAT_draw_sprite(&mines_sprite, 10, x * CAT_TILE_SIZE, (GRID_HEIGHT-1) * CAT_TILE_SIZE);
		}
		for(int y = 0; y < GRID_HEIGHT; y++)
		{
			CAT_draw_sprite(&mines_sprite, 10, 0, y * CAT_TILE_SIZE);
			CAT_draw_sprite(&mines_sprite, 10, (GRID_WIDTH-1) * CAT_TILE_SIZE, y * CAT_TILE_SIZE);
		}

		if(state == WIN)
		{
			CAT_gui_panel((CAT_ivec2) {1, 1}, (CAT_ivec2) {13, 18});
			CAT_gui_set_flag(CAT_GUI_TEXT_WRAP);
			CAT_gui_text("All Clear!\nThe fields are safe.\nFor your diligent work, you've earned a commemorative prop.\nFind it in your bag!");
			CAT_gui_line_break();
			CAT_gui_line_break();
			CAT_gui_image(&prop_mine_sprite, 0);
			CAT_gui_text(" +1");
		}
		else
		{
			CAT_gui_panel((CAT_ivec2) {1, 1}, (CAT_ivec2) {13, 18});
			CAT_gui_set_flag(CAT_GUI_TEXT_WRAP);
			CAT_gui_text("Kaboom!\n\nYour exploration has come to an explosive end.\n\nPress A or B to return from whence you came.");
		}	
	}
}