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
#define ENEMY_VULN_FRAMES 3
#define ENEMY_HITSTOP_FRAMES 2

static int enemy_x[ENEMY_CAPACITY];
static int enemy_y[ENEMY_CAPACITY];
static int8_t enemy_tx[ENEMY_CAPACITY];
static int8_t enemy_ty[ENEMY_CAPACITY];
static uint8_t enemy_health[ENEMY_CAPACITY];
static bool enemy_hit[ENEMY_CAPACITY];
static uint8_t enemy_vuln[ENEMY_CAPACITY];
static uint8_t enemy_count = 0;

void enemy_swap(int i, int j)
{
	int temp = enemy_x[i];
	enemy_x[i] = enemy_x[j];
	enemy_x[j] = temp;
	temp = enemy_y[i];
	enemy_y[i] = enemy_y[j];
	enemy_y[j] = temp;
	temp = enemy_tx[i];
	enemy_tx[i] = enemy_tx[j];
	enemy_tx[j] = temp;
	temp = enemy_ty[i];
	enemy_ty[i] = enemy_ty[j];
	enemy_ty[j] = temp;
	temp = enemy_health[i];
	enemy_health[i] = enemy_health[j];
	enemy_health[j] = temp;
	temp = enemy_hit[i];
	enemy_hit[i] = enemy_hit[j];
	enemy_hit[j] = temp;
}

void CAT_spawn_enemy(int x, int y)
{
	if(enemy_count >= ENEMY_CAPACITY)
		return;
	int idx = enemy_count;
	enemy_x[idx] = x;
	enemy_y[idx] = y;
	enemy_tx[idx] = CAT_rand_int(-1, 1);
	enemy_ty[idx] = CAT_rand_int(-1, 1);
	enemy_health[idx] = ENEMY_HEALTH;
	enemy_hit[idx] = false;
	enemy_count += 1;
}

void hit_enemy(int idx)
{
	if(!enemy_hit[idx])
	{
		enemy_health[idx] = clamp(enemy_health[idx]-1, 0, ENEMY_HEALTH);
		enemy_vuln[idx] = ENEMY_VULN_FRAMES;
	}
	enemy_hit[idx] = true;
}

void kill_enemy(int idx)
{
	enemy_count -= 1;
	enemy_swap(enemy_count, idx);
}

bool is_enemy_alive(int idx)
{
	return enemy_health[idx] > 0;
}

void CAT_tick_enemies()
{
	for(int i = 0; i < enemy_count; i++)
	{
		if
		(
			(!CAT_rect_point_intersect
			(
				0, 0,
				CAT_LCD_SCREEN_W, CAT_LCD_SCREEN_H,
				enemy_x[i], enemy_y[i]
			) ||
			enemy_health[i] <= 0) &&
			enemy_vuln[i] <= 0
		)
		{
			kill_enemy(i);
			i -= 1;
			continue;
		}

		int player_x, player_y;
		CAT_world_get_position(&player_x, &player_y);
		enemy_tx[i] = sgn(player_x - enemy_x[i]);
		enemy_ty[i] = sgn(player_y - enemy_y[i]);

		if(enemy_health[i] > 0 && (!enemy_hit[i] || (ENEMY_VULN_FRAMES - enemy_vuln[i]) > ENEMY_HITSTOP_FRAMES))
		{
			float dt = CAT_get_delta_time_s();
			enemy_x[i] += enemy_tx[i] * ENEMY_SPEED * dt;
			enemy_y[i] += enemy_ty[i] * ENEMY_SPEED * dt;
		}

		if(enemy_hit[i])
		{
			if(enemy_vuln[i] == 0)
				enemy_hit[i] = false;
			else
				enemy_vuln[i] -= 1;
		}
	}
}

void CAT_render_enemies()
{
	for(int i = 0; i < enemy_count; i++)
	{
		CAT_circberry(enemy_x[i], enemy_y[i], 4, CAT_RED);
		
		CAT_fillberry(enemy_x[i]-ENEMY_SIZE/2, enemy_y[i]-ENEMY_SIZE/2-3, ENEMY_SIZE * enemy_health[i] / (float) ENEMY_HEALTH, 2, CAT_GREEN);
		bool flash = enemy_hit[i] && (enemy_vuln[i] & 1);
		CAT_strokeberry(enemy_x[i]-ENEMY_SIZE/2, enemy_y[i]-ENEMY_SIZE/2, ENEMY_SIZE, ENEMY_SIZE, flash ? CAT_RED : CAT_BLACK);

		CAT_set_text_colour(CAT_WHITE);
		CAT_draw_textf(enemy_x[i] + ENEMY_SIZE/2 + 2, enemy_y[i] - ENEMY_SIZE/2, "%.2d/%.2d", i, enemy_count);
	}
}

#define BULLET_CAPACITY 128
#define BULLET_R 4

static int bullet_x[BULLET_CAPACITY];
static int bullet_y[BULLET_CAPACITY];
static int8_t bullet_tx[BULLET_CAPACITY];
static int8_t bullet_ty[BULLET_CAPACITY];
static uint8_t bullet_speed[BULLET_CAPACITY];
static uint8_t bullet_count = 0;

void bullet_swap(int i, int j)
{
	int temp = bullet_x[i];
	bullet_x[i] = bullet_x[j];
	bullet_x[j] = temp;
	temp = bullet_y[i];
	bullet_y[i] = bullet_y[j];
	bullet_y[j] = temp;
	temp = bullet_tx[i];
	bullet_tx[i] = bullet_tx[j];
	bullet_tx[j] = temp;
	temp = bullet_ty[i];
	bullet_ty[i] = bullet_ty[j];
	bullet_ty[j] = temp;
	temp = bullet_speed[i];
	bullet_speed[i] = bullet_speed[j];
	bullet_speed[j] = temp;
}

void CAT_attack_bullet(int x, int y, int tx, int ty, int speed)
{
	if(bullet_count >= BULLET_CAPACITY)
		return;
	int idx = bullet_count;
	bullet_x[idx] = x;
	bullet_y[idx] = y;
	bullet_tx[idx] = clamp(tx, -127, 127);
	bullet_ty[idx] = clamp(ty, -127, 127);
	bullet_speed[idx] = clamp(speed, 0, 255);
	bullet_count += 1;
}

void kill_bullet(int idx)
{
	bullet_count -= 1;
	bullet_swap(bullet_count, idx);
}

void tick_bullets()
{
	for(int i = 0; i < bullet_count; i++)
	{
		if(!CAT_rect_point_intersect(
			0, 0,
			CAT_LCD_SCREEN_W, CAT_LCD_SCREEN_H,
			bullet_x[i], bullet_y[i]
		))
		{
			kill_bullet(i);
			i -= 1;
			continue;
		}

		float dt = CAT_get_delta_time_s();
		bullet_x[i] += bullet_tx[i] * bullet_speed[i] * dt;
		bullet_y[i] += bullet_ty[i] * bullet_speed[i] * dt;

		bool hit = false;
		for(int j = 0; j < enemy_count; j++)
		{
			if(CAT_rect_point_intersect(
				enemy_x[j]-ENEMY_SIZE/2, enemy_y[j]-ENEMY_SIZE/2,
				enemy_x[j]+ENEMY_SIZE/2, enemy_y[j]+ENEMY_SIZE/2,
				bullet_x[i], bullet_y[i]
			))
			{
				hit_enemy(j);
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
		CAT_circberry(bullet_x[i], bullet_y[i], 4, CAT_WHITE);
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
		if(CAT_rect_point_intersect(
			enemy_x[i] - ENEMY_SIZE/2, enemy_y[i] - ENEMY_SIZE/2,
			enemy_x[i] + ENEMY_SIZE/2, enemy_y[i] + ENEMY_SIZE/2,
			swipe_x, swipe_y
		))
		{
			hit_enemy(i);
			return;
		}

		int d_x = enemy_x[i] - swipe_x;
		int d_y = enemy_y[i] - swipe_y;
		int d2 = CAT_i2_cross(d_x, d_y, d_x, d_y);
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

		hit_enemy(i);
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
	int x_l = swipe_x + SWIPE_R * cos(t_l);
	int y_l = swipe_y + SWIPE_R * sin(t_l);
	int x_r = swipe_x + SWIPE_R * cos(t_r);
	int y_r = swipe_y + SWIPE_R * sin(t_r);
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