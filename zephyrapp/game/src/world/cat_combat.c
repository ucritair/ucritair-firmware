#include "cat_combat.h"

#include <stdint.h>
#include "cat_math.h"
#include "cat_core.h"
#include "cat_render.h"
#include <math.h>
#include "cat_gui.h"
#include "cat_world.h"

#define ENEMY_CAPACITY 64
#define ENEMY_HEALTH 10
#define ENEMY_SPEED 24
#define ENEMY_SIZE 24
#define ENEMY_INVULN_FRAMES 3
#define ENEMY_HITSTOP_FRAMES 2

typedef struct
{
	int x;
	int y;
	int8_t tx;
	int8_t ty;

	uint8_t health;
	bool hit;
	uint8_t invuln;
} CAT_enemy;

static CAT_enemy enemies[ENEMY_CAPACITY];
static uint8_t enemy_count = 0;

void CAT_spawn_enemy(int x, int y)
{
	if(enemy_count >= ENEMY_CAPACITY)
		return;
	int idx = enemy_count;
	enemies[idx] = (CAT_enemy)
	{
		.x = x, .y = y,
		.tx = 0, .ty = 1,
		.health = ENEMY_HEALTH,
		.hit = false,
		.invuln = 0
	};
	enemy_count += 1;
}

void hit_enemy(int idx, int damage, int8_t vx, int8_t vy)
{
	if(!enemies[idx].hit)
	{
		enemies[idx].x += vx;
		enemies[idx].y += vy;

		enemies[idx].health = clamp(enemies[idx].health-damage, 0, ENEMY_HEALTH);
		enemies[idx].invuln = ENEMY_INVULN_FRAMES;
	}
	enemies[idx].hit = true;
}

void kill_enemy(int idx)
{
	enemy_count -= 1;
	CAT_enemy temp = enemies[enemy_count];
	enemies[enemy_count] = enemies[idx];
	enemies[idx] = temp;
}

bool is_enemy_alive(int idx)
{
	return enemies[idx].health > 0;
}

void CAT_tick_enemies()
{
	for(int i = 0; i < enemy_count; i++)
	{
		if
		(
			(!CAT_rect_point_touching
			(
				0, 0,
				CAT_LCD_SCREEN_W, CAT_LCD_SCREEN_H,
				enemies[i].x, enemies[i].y
			) ||
			enemies[i].health <= 0) &&
			enemies[i].invuln <= 0
		)
		{
			kill_enemy(i);
			i -= 1;
			continue;
		}

		int player_x, player_y;
		CAT_world_get_position(&player_x, &player_y);
		enemies[i].tx = sgn(player_x - enemies[i].x);
		enemies[i].ty = sgn(player_y - enemies[i].y);

		if(enemies[i].health > 0 && (!enemies[i].hit || (ENEMY_INVULN_FRAMES - enemies[i].invuln) > ENEMY_HITSTOP_FRAMES))
		{
			float dt = CAT_get_delta_time_s();
			enemies[i].x += enemies[i].tx * ENEMY_SPEED * dt;
			enemies[i].y += enemies[i].ty * ENEMY_SPEED * dt;
		}

		if(enemies[i].hit)
		{
			if(enemies[i].invuln == 0)
				enemies[i].hit = false;
			else
				enemies[i].invuln -= 1;
		}
	}
}

void CAT_render_enemies()
{
	for(int i = 0; i < enemy_count; i++)
	{
		CAT_circberry(enemies[i].x, enemies[i].y, 4, CAT_RED);
		
		CAT_fillberry(enemies[i].x-ENEMY_SIZE/2, enemies[i].y-ENEMY_SIZE/2-3, ENEMY_SIZE * enemies[i].health / (float) ENEMY_HEALTH, 2, CAT_GREEN);
		bool flash = enemies[i].hit && (enemies[i].invuln & 1);
		CAT_strokeberry(enemies[i].x-ENEMY_SIZE/2, enemies[i].y-ENEMY_SIZE/2, ENEMY_SIZE, ENEMY_SIZE, flash ? CAT_RED : CAT_BLACK);

		CAT_set_text_colour(CAT_WHITE);
		CAT_draw_textf(enemies[i].x + ENEMY_SIZE/2 + 2, enemies[i].y - ENEMY_SIZE/2, "%.2d/%.2d", i, enemy_count);
	}
}

#define BULLET_CAPACITY 128
#define BULLET_R 4

typedef struct
{
	int x;
	int y;
	int8_t tx;
	int8_t ty;
	uint8_t speed;
} CAT_bullet;

static CAT_bullet bullets[BULLET_CAPACITY];
static uint8_t bullet_count = 0;

void CAT_attack_bullet(int x, int y, int tx, int ty, int speed)
{
	if(bullet_count >= BULLET_CAPACITY)
		return;
	int idx = bullet_count;
	bullets[idx] = (CAT_bullet)
	{
		.x = x, .y = y,
		.tx = clamp(tx, -127, 127), .ty = clamp(ty, -127, 127),
		.speed = clamp(speed, 0, 255)
	};
	bullet_count += 1;
}

void kill_bullet(int idx)
{
	bullet_count -= 1;
	CAT_bullet temp = bullets[bullet_count];
	bullets[bullet_count] = bullets[idx];
	bullets[idx] = temp;
}

void tick_bullets()
{
	for(int i = 0; i < bullet_count; i++)
	{
		if(!CAT_rect_point_touching(
			0, 0,
			CAT_LCD_SCREEN_W, CAT_LCD_SCREEN_H,
			bullets[i].x, bullets[i].y
		))
		{
			kill_bullet(i);
			i -= 1;
			continue;
		}

		float dt = CAT_get_delta_time_s();
		bullets[i].x += bullets[i].tx * bullets[i].speed * dt;
		bullets[i].y += bullets[i].ty * bullets[i].speed * dt;

		bool hit = false;
		for(int j = 0; j < enemy_count; j++)
		{
			if(CAT_rect_point_touching(
				enemies[j].x-ENEMY_SIZE/2, enemies[j].y-ENEMY_SIZE/2,
				enemies[j].x+ENEMY_SIZE/2, enemies[j].y+ENEMY_SIZE/2,
				bullets[i].x, bullets[i].y
			))
			{
				hit_enemy(j, 1, 4 * bullets[i].tx, 4 * bullets[i].ty);
				hit = true;
				break;
			}
		}
		if(hit)
		{
			kill_bullet(i);
			i -= 1;
		}
	}
}

void render_bullets()
{
	for(int i = 0; i < bullet_count; i++)
	{
		CAT_circberry(bullets[i].x, bullets[i].y, 4, CAT_WHITE);
	}
}

#define SWIPE_ARC (M_PI * 0.35)
#define SWIPE_R 48
#define SWIPE_DURATION 1
#define SWIPE_COOLDOWN 1

static int swipe_x = 0;
static int swipe_y = 0;
static int8_t swipe_tx = 0;
static int8_t swipe_ty = 0;
static bool swiping = false;
static uint8_t swipe_frames = 0;

// tx+1, ty+1
static float swipe_angles[] =
{
	M_PI + M_PI / 4, // -1, -1 (SW)
	M_PI, // -1, 0 (W)
	M_PI / 2 + M_PI / 4, // -1, 1 (NW)
	3 * M_PI / 2, // 0, -1 (S)
	0, // 0, 0 (None)
	M_PI / 2, // 0, 1 (N)
	3 * M_PI / 2 + M_PI / 4, // 1, -1 (SE)
	0, // 1, 0  (E)
	M_PI / 4, // 1, 1 (NE)
};

void CAT_attack_swipe(int x, int y, int tx, int ty)
{
	if(swiping)
		return;
	if(swipe_frames < SWIPE_COOLDOWN)
		return;
	swipe_x = x;
	swipe_y = y;
	swipe_tx = clamp(tx, -127, 127);
	swipe_ty = clamp(ty, -127, 127);
	swiping = true;
	swipe_frames = 0;

	for(int i = 0; i < enemy_count; i++)
	{
		if(CAT_rect_point_touching(
			enemies[i].x - ENEMY_SIZE/2, enemies[i].y - ENEMY_SIZE/2,
			enemies[i].x + ENEMY_SIZE/2, enemies[i].y + ENEMY_SIZE/2,
			swipe_x, swipe_y
		))
		{
			hit_enemy(i, 2, 24 * swipe_tx, 24 * swipe_ty);
			return;
		}

		int d_x = enemies[i].x - swipe_x;
		int d_y = enemies[i].y - swipe_y;
		int d2 = CAT_i2_dot(d_x, d_y, d_x, d_y);
		if(d2 > SWIPE_R*SWIPE_R)
			continue;
		
		int angle_idx = (swipe_tx+1) * 3 + (swipe_ty+1);
		float t_s = swipe_angles[angle_idx];
		float t_e = atan2(d_y, d_x);
		if(t_e < 0)
			t_e += 2 * M_PI;
		float d_t = fabs(t_e - t_s);
		if(d_t > SWIPE_ARC)
			continue;

		hit_enemy(i, 2, 12 * swipe_tx, 12 * swipe_ty);
	}
}

void tick_swipe()
{
	swipe_frames += 1;

	if(swiping)
	{
		if(swipe_frames > SWIPE_DURATION)
		{
			swiping = false;
			swipe_frames = 0;
		}
	}
}

void render_swipe()
{
	if(!swiping)
		return;
	int angle_idx = (swipe_tx+1) * 3 + (swipe_ty+1);
	float t = swipe_angles[angle_idx];
	float t_l = t - SWIPE_ARC / 2;
	float t_r = t + SWIPE_ARC / 2;
	int x_l = swipe_x + SWIPE_R * cosf(t_l);
	int y_l = swipe_y + SWIPE_R * sinf(t_l);
	int x_r = swipe_x + SWIPE_R * cosf(t_r);
	int y_r = swipe_y + SWIPE_R * sinf(t_r);
	CAT_lineberry(swipe_x, swipe_y, x_l, y_l, CAT_WHITE);
	CAT_lineberry(swipe_x, swipe_y, x_r, y_r, CAT_WHITE);
	CAT_lineberry(x_l, y_l, x_r, y_r, CAT_WHITE);
}

void CAT_tick_attacks()
{
	tick_bullets();
	tick_swipe();
}

void CAT_render_attacks()
{
	render_bullets();
	render_swipe();
}