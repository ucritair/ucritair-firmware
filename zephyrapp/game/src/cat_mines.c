#include "cat_arcade.h"

#include "cat_input.h"
#include "cat_sprite.h"
#include "cat_room.h"
#include "cat_bag.h"
#include "cat_gui.h"

#define GRID_WIDTH 15
#define GRID_HEIGHT 20
#define GRID_SIZE (GRID_WIDTH * GRID_HEIGHT)
#define MINE_COUNT (GRID_SIZE / 5)

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
	return !cell->visited;
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

static bool dead = false;
static CAT_ivec2 cursor = {0, 0};
static bool first_click = true;
static int clicks = 0;

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
				neighbour->adjacent += da;
		}
	}
}

void flood_reveal(int x, int y)
{
	idx_queue_length = 0;
	idx_enqueue(y * GRID_WIDTH + x);
	while(idx_queue_length > 0)
	{		
		grid_cell* c = &grid[idx_dequeue()];
		c->seen = true;
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

void CAT_MS_mines(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
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

						.adjacent = 0
					};
				}
			}
			
			for(int i = 0; i < MINE_COUNT; i++)
			{
				int x = CAT_rand_int(0, GRID_WIDTH-1);
				int y = CAT_rand_int(0, GRID_HEIGHT-1);
				toggle_mine(x, y, true);
			}

			dead = false;
			first_click = true;
			clicks = 0;

			if(reveal_timer_id == -1)
				reveal_timer_id = CAT_timer_init(0.05f);
			reveal_complete = false;
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{	
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_back();
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_room);

			if(!dead)
			{
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

				if(CAT_input_pressed(CAT_BUTTON_A))
				{
					if(first_click)
					{
						for(int dy = -1; dy <= 1; dy++)
						{
							for(int dx = -1; dx <= 1; dx++)
							{
								grid_cell* cand = get_cell(cursor.x+dx, cursor.y+dy);
								if(cand != NULL && cand->mine)
									toggle_mine(cursor.x+dx, cursor.y+dy, false);
							}
						}
						first_click = false;
					}
					
					if(!cell->seen)
					{
						cell->seen = true;

						if(cell->mine)
						{
							dead = true;
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
					}				
				}
				if(CAT_input_pressed(CAT_BUTTON_SELECT))
					cell->flagged = !cell->flagged;
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
						CAT_machine_back();
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
		spriter.mode = CAT_DRAW_MODE_DEFAULT;

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
						CAT_draw_sprite(mines_sprite, 10, xs, ys);
					}
					else
					{
						CAT_draw_sprite(mines_sprite, 1, xs, ys);
						if(cell->coin)
							CAT_draw_sprite(coin_world_sprite, 0, xs, ys);
						if(cell->adjacent > 0)
							CAT_draw_sprite(mines_sprite, 1 + cell->adjacent, xs, ys);
					}
				}
				else
				{
					CAT_draw_sprite(mines_sprite, 0, xs, ys);
					if(cell->flagged)
						CAT_draw_sprite(mines_sprite, 12, xs, ys);
				}
			}
		}
		if(!dead)
			CAT_draw_sprite(mines_sprite, 11, cursor.x*CAT_TILE_SIZE, cursor.y*CAT_TILE_SIZE);
	}
	else
	{
		CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 20});

		spriter.mode = CAT_DRAW_MODE_DEFAULT;
		for(int x = 0; x < GRID_WIDTH; x++)
		{
			CAT_draw_sprite(mines_sprite, 10, x * CAT_TILE_SIZE, 0);
			CAT_draw_sprite(mines_sprite, 10, x * CAT_TILE_SIZE, (GRID_HEIGHT-1) * CAT_TILE_SIZE);
		}
		for(int y = 0; y < GRID_HEIGHT; y++)
		{
			CAT_draw_sprite(mines_sprite, 10, 0, y * CAT_TILE_SIZE);
			CAT_draw_sprite(mines_sprite, 10, (GRID_WIDTH-1) * CAT_TILE_SIZE, y * CAT_TILE_SIZE);
		}

		CAT_gui_panel((CAT_ivec2) {1, 1}, (CAT_ivec2) {13, 18});
		CAT_gui_set_flag(CAT_GUI_WRAP_TEXT);
		CAT_gui_text("Kaboom!\n\nYour exploration has come to an explosive end.\n\nPress A or B to return from whence you came.");
	}
}