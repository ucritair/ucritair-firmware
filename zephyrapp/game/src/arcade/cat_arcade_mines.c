#include "cat_arcade.h"

#include "cat_input.h"
#include "cat_render.h"
#include "cat_room.h"
#include "cat_item.h"
#include "cat_gui.h"
#include "sprite_assets.h"
#include "sound_assets.h"
#include "item_assets.h"

#define GRID_WIDTH 15
#define GRID_HEIGHT 20
#define GRID_SIZE (GRID_WIDTH * GRID_HEIGHT)
#define MINE_COUNT (GRID_SIZE / 7)

typedef struct grid_cell
{
	int x;
	int y;
	uint16_t idx;

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
static uint16_t idx_queue[GRID_SIZE];
static int idx_queue_length = 0;

static void idx_enqueue(uint16_t idx)
{
	if(idx_queue_length >= GRID_SIZE)
		return;

	idx_queue[idx_queue_length] = idx;
	idx_queue_length += 1;
}

static uint16_t idx_dequeue()
{
	if(idx_queue_length <= 0)
		return -1;

	uint16_t idx = idx_queue[0];
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

static int last_revealed = 0;
static bool reveal_complete = false;

static int tbv = 0;
static int score = 0;

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
			uint16_t idx = y * GRID_WIDTH + x;
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
	for(uint16_t i = 0; i < GRID_SIZE; i++)
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
				uint16_t nidx = (c->y+dy) * CAT_ROOM_GRID_W + (c->x+dx);
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

void flood_tbv(int x, int y)
{
	idx_queue_length = 0;
	idx_enqueue(y * GRID_WIDTH + x);
	
	while(idx_queue_length > 0)
	{		
		grid_cell* c = &grid[idx_dequeue()];
		c->visited = true;

		for(int dy = -1; dy <= 1; dy++)
		{
			for(int dx = -1; dx <= 1; dx++)
			{
				if(dx == 0 && dy == 0)
					continue;
				int xp = c->x+dx;
				int yp = c->y+dy;
				if(xp < 0 || xp >= GRID_WIDTH || yp < 0 || yp >= GRID_HEIGHT)
					continue;

				uint16_t nidx = yp * CAT_ROOM_GRID_W + xp;
				if(!grid[nidx].visited && !grid[nidx].mine)
					idx_enqueue(nidx);
			}
		}
	}
}

void calculate_tbv()
{
	tbv = 0;
	for(int i = 0; i < GRID_SIZE; i++)
		grid[i].visited = false;

	for(int y = 0; y < GRID_HEIGHT; y++)
	{
		for(int x = 0; x < GRID_WIDTH; x++)
		{
			int idx = y * GRID_WIDTH + x;
			if(!grid[idx].mine && !grid[idx].visited)
			{
				tbv += 1;
				flood_tbv(x, y);
			}
		}
	}
	for(int i = 0; i < GRID_SIZE; i++)
	{
		if(!grid[i].mine && !grid[i].visited)
			tbv += 1;
		grid[i].visited = false;
	}
}

void CAT_MS_mines(CAT_FSM_signal signal)
{
	switch(signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		{		
			CAT_set_render_callback(CAT_render_mines);

			init_grid();

			state = PLAY;
			first_click = true;
			clicks = 0;
			cursor = (CAT_ivec2) {0, 0};

			last_revealed = -1;
			reveal_complete = false;

			tbv = 0;
			score = 0;
			calculate_tbv();
			break;
		}
		case CAT_FSM_SIGNAL_TICK:
		{	
			if(state == PLAY)
			{
				if(CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
						CAT_gui_open_popup("Quit Sweep?\n\nProgress will not be saved!\n\n", CAT_POPUP_STYLE_YES_NO);
				else if(CAT_gui_consume_popup())
				{
					CAT_pushdown_pop();
					break;
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
				cursor.x = CAT_clamp(cursor.x, 0, GRID_WIDTH-1);
				cursor.y = CAT_clamp(cursor.y, 0, GRID_HEIGHT-1);
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
							score = ((GRID_SIZE-unseen) / (float) GRID_SIZE) * 100.0f;
							break;
						}

						clicks += 1;
						if(clicks == 8)
						{
							CAT_inventory_add(coin_item, 1);
							cell->coin = true;
							clicks = 0;
						}

						flood_reveal(cursor.x, cursor.y);

						if(unseen == MINE_COUNT)
						{
							state = WIN;
							score = (tbv / (float) clicks) * 100;
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
					if(CAT_input_dismissal() || CAT_input_pressed(CAT_BUTTON_A))
						reveal_complete = true;

					for(int i = last_revealed; i < GRID_SIZE; i++)
					{
						if(grid[i].mine && !grid[i].seen)
						{
							grid[i].seen = true;
							last_revealed = i;
							break;
						}
						else if(i == GRID_SIZE-1)
							reveal_complete = true;
					}
				}
				else
				{
					if(CAT_input_dismissal() || CAT_input_pressed(CAT_BUTTON_A))
					{
						if(state == WIN)
							CAT_inventory_add(prop_mine_item, 1);
						CAT_pushdown_pop();
					}
				}
				break;
			}

			break;
		}
		case CAT_FSM_SIGNAL_EXIT:
		{
			if(score > mines_highscore)
				mines_highscore = score;
			break;
		}
	}
}

void CAT_render_mines()
{
	if(!reveal_complete)
	{
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
						CAT_draw_tile_alpha(&mines_sprite, 10, xs, ys);
					}
					else
					{
						CAT_draw_tile_alpha(&mines_sprite, 1, xs, ys);
						if(cell->adjacent > 0)
							CAT_draw_tile_alpha(&mines_sprite, 1 + cell->adjacent, xs, ys);
						if(cell->coin && cell->coin_timer <= 1.0f)
							CAT_draw_tile_alpha(&coin_world_sprite, 0, xs, ys);
					}
				}
				else
				{
					CAT_draw_tile_alpha(&mines_sprite, 0, xs, ys);
					if(cell->flagged)
						CAT_draw_tile_alpha(&mines_sprite, 12, xs, ys);
				}
			}
		}
		if(state == PLAY)
			CAT_draw_tile_alpha(&mines_sprite, 11, cursor.x*CAT_TILE_SIZE, cursor.y*CAT_TILE_SIZE);
	}
	else
	{
		CAT_frameberry(CAT_WHITE);

		for(int x = 0; x < GRID_WIDTH; x++)
		{
			CAT_draw_tile_alpha(&mines_sprite, 10, x * CAT_TILE_SIZE, 0);
			CAT_draw_tile_alpha(&mines_sprite, 10, x * CAT_TILE_SIZE, (GRID_HEIGHT-1) * CAT_TILE_SIZE);
		}
		for(int y = 0; y < GRID_HEIGHT; y++)
		{
			CAT_draw_tile_alpha(&mines_sprite, 10, 0, y * CAT_TILE_SIZE);
			CAT_draw_tile_alpha(&mines_sprite, 10, (GRID_WIDTH-1) * CAT_TILE_SIZE, y * CAT_TILE_SIZE);
		}

		if(state == WIN)
		{
			int pad = CAT_TILE_SIZE + 4;
			CAT_set_text_mask(pad, -1, CAT_LCD_SCREEN_W-pad, -1);
			CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
			int cursor_y = pad;
			cursor_y = CAT_draw_text(pad, cursor_y, "All clear! The fields are safe. For your diligent work, you've earned a commemorative prop. Find it in your bag!\n\n");

			CAT_draw_sprite(&prop_mine_sprite, 0, pad, cursor_y);
			cursor_y += prop_mine_sprite.height / 2;
			CAT_draw_text(pad + prop_mine_sprite.width, cursor_y, "+1");
			cursor_y += prop_mine_sprite.height / 2;
			cursor_y += 24;

			cursor_y = CAT_draw_textf
			(
				pad, cursor_y,
				"Score: %d\n"
				"High score: %d\n",
				score, mines_highscore
			);
			if(score > mines_highscore)
				cursor_y = CAT_draw_textf(pad, cursor_y, "NEW HIGH SCORE!\n");
		}
		else
		{			
			int pad = CAT_TILE_SIZE + 4;
			CAT_set_text_mask(pad, -1, CAT_LCD_SCREEN_W-pad, -1);
			CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
			int cursor_y = pad;
			cursor_y = CAT_draw_text(pad, cursor_y, "Kaboom!\nYour exploration has come to an explosive end. Press A or B to return from whence you came.\n\n");

			cursor_y = CAT_draw_textf
			(
				pad, cursor_y,
				"Score: %d\n"
				"High score: %d\n",
				score, mines_highscore
			);
			if(score > mines_highscore)
				cursor_y = CAT_draw_textf(pad, cursor_y, "NEW HIGH SCORE!\n");
		}
	}
}