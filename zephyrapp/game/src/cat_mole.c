#include "cat_mole.h"

#include "cat_input.h"
#include "cat_sprite.h"
#include "cat_room.h"
#include "cat_arcade.h"
#include "cat_bag.h"
#include "cat_gui.h"

#define GRID_WIDTH 15
#define GRID_HEIGHT 20
#define GRID_SIZE (GRID_WIDTH * GRID_HEIGHT)

int mole_high_score = 0;
static int score = 0;
static bool is_high_score = false;

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

typedef struct grid_cell
{
	int x;
	int y;
	int idx;

	bool mine;
	bool seen;
	bool flagged;
	bool visited;

	int adjacent;
} grid_cell;
static grid_cell grid[GRID_SIZE];
static int mine_count = GRID_SIZE / 5;

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

void CAT_MS_mole(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			score = 0;
			is_high_score = false;

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

						.adjacent = 0
					};
				}
			}

			for(int i = 0; i < mine_count; i++)
			{
				int x = CAT_rand_int(0, GRID_WIDTH-1);
				int y = CAT_rand_int(0, GRID_HEIGHT-1);
				grid_cell* cell = get_cell(x, y);
				cell->mine = true;
				for(int dy = -1; dy <= 1; dy++)
				{
					for(int dx = -1; dx <= 1; dx++)
					{
						grid_cell* neighbour = get_cell(x+dx, y+dy);
						if(neighbour != NULL)
							neighbour->adjacent += 1;
					}
				}
			}
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{	
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_back();
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_room);
			
			if(CAT_input_touching())
			{
				for(int y = 0; y < GRID_HEIGHT; y++)
				{
					for(int x = 0; x < GRID_WIDTH; x++)
					{
						int xs = x * CAT_TILE_SIZE;
						int ys = y * CAT_TILE_SIZE;
						if(CAT_input_touch_rect(xs+1, ys+1, CAT_TILE_SIZE-1, CAT_TILE_SIZE-1))
						{
							get_cell(x, y)->seen = true;

							idx_queue_length = 0;
							idx_enqueue(y * GRID_WIDTH + x);

							while(idx_queue_length > 0)
							{		
								grid_cell* c = &grid[idx_dequeue()];
								c->seen = true;
								if(c->adjacent > 0)
									continue;

								CAT_ivec2 n = {c->x, c->y-1};
								int n_idx = n.y * CAT_GRID_WIDTH + n.x;
								if(is_cell_clean(n.x, n.y))
								{
									grid[n_idx].visited = true;
									idx_enqueue(n_idx);
								}
								CAT_ivec2 e = {c->x+1, c->y};
								int e_idx = e.y * CAT_GRID_WIDTH + e.x;
								if(is_cell_clean(e.x, e.y))
								{
									grid[e_idx].visited = true;
									idx_enqueue(e_idx);
								}
								CAT_ivec2 s = {c->x, c->y+1};
								int s_idx = s.y * CAT_GRID_WIDTH + s.x;
								if(is_cell_clean(s.x, s.y))
								{
									grid[s_idx].visited = true;
									idx_enqueue(s_idx);
								}	
								CAT_ivec2 w = {c->x-1, c->y};
								int w_idx = w.y * CAT_GRID_WIDTH + w.x;
								if(is_cell_clean(w.x, w.y))
								{
									grid[w_idx].visited = true;
									idx_enqueue(w_idx);
								}
							}
						}
					}
				}
			}
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			break;
		}
	}
}

void CAT_render_mole()
{
	spriter.mode = CAT_DRAW_MODE_DEFAULT;

	for(int y = 0; y < GRID_HEIGHT; y++)
	{
		for(int x = 0; x < GRID_WIDTH; x++)
		{
			grid_cell* cell = get_cell(x, y);
			if(cell->seen)
			{
				if(cell->mine)
				{
					CAT_draw_sprite(mines_sprite, 10, x*CAT_TILE_SIZE, y*CAT_TILE_SIZE);
				}
				else
				{
					CAT_draw_sprite(mines_sprite, 1, x*CAT_TILE_SIZE, y*CAT_TILE_SIZE);
					CAT_draw_sprite(mines_sprite, 1 + cell->adjacent, x*CAT_TILE_SIZE, y*CAT_TILE_SIZE);
				}
			}
			else
			{
				CAT_draw_sprite(mines_sprite, 0, x*CAT_TILE_SIZE, y*CAT_TILE_SIZE);
			}	
		}
	}
}