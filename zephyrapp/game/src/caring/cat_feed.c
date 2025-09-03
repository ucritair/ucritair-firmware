#include "cat_actions.h"

#include "cat_math.h"
#include "cat_input.h"
#include "cat_render.h"
#include "sprite_assets.h"
#include "item_assets.h"
#include "cat_item.h"
#include "cat_item.h"
#include "cowtools/cat_structures.h"
#include "cat_gui.h"
#include "cowtools/cat_curves.h"
#include "cat_pet.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "cat_room.h"
#include "cat_gizmos.h"

static void MS_feed_arrange(CAT_machine_signal signal);
static void render_arrange();
static void MS_feed_select(CAT_machine_signal signal);
static void render_select();
static void MS_feed_inspect(CAT_machine_signal signal);
static void render_inspect();
static void MS_feed_summary(CAT_machine_signal signal);
static void render_summary();

#define FOOD_SCALE 2
#define FOOD_COLLISION_R 16
#define FOOD_COLLISION_W (FOOD_SCALE * 16)
#define FOOD_COLLISION_H (FOOD_SCALE * 16)
#define MAX_FOOD_COUNT 5

#define COUNTER_X 0
#define COUNTER_Y 0
#define COUNTER_W 240
#define COUNTER_H 80

#define TABLE_X 60
#define TABLE_Y 140
#define TABLE_W 120
#define TABLE_H 68

#define SPAWN_X 16
#define SPAWN_Y 16
#define SPAWN_W FOOD_COLLISION_W
#define SPAWN_H FOOD_COLLISION_H
#define SPAWN_MARGIN 12

#define CENTERPIECE_RADIUS 12

#define SELECT_GRID_MARGIN 12

static CAT_rect counter_rect =
{
	.min = {COUNTER_X, COUNTER_Y},
	.max = {COUNTER_X + COUNTER_W, COUNTER_Y + COUNTER_H}
};
static CAT_rect table_rect =
{
	.min = {TABLE_X, TABLE_Y},
	.max = {TABLE_X + TABLE_W, TABLE_Y + TABLE_H}
};
static CAT_rect spawn_rects[MAX_FOOD_COUNT];

static void init_spawn_rects()
{
	for (int i = 0; i < 5; i++)
	{
		spawn_rects[i] = (CAT_rect){
			.min = (CAT_ivec2){SPAWN_X + SPAWN_W * i + SPAWN_MARGIN * i, SPAWN_Y},
			.max = (CAT_ivec2){SPAWN_X + SPAWN_W * i + SPAWN_MARGIN * i + SPAWN_W, SPAWN_Y + SPAWN_H},
		};
	}
}

static int food_pool_backing[CAT_ITEM_TABLE_CAPACITY];
static CAT_int_list food_pool;

static void init_item_id_pool()
{
	CAT_ilist(&food_pool, food_pool_backing, CAT_ITEM_TABLE_CAPACITY);
	for (int i = 0; i < item_table.length; i++)
	{
		if 
		(
			item_table.counts[i] > 0 &&
			item_table.data[i].type == CAT_ITEM_TYPE_TOOL &&
			item_table.data[i].tool_type == CAT_TOOL_TYPE_FOOD
		)
		{
			CAT_ilist_push(&food_pool, i);
		}
	}
	CAT_ilist_shuffle(&food_pool);
}

typedef struct
{
	int pool_idx;

	CAT_ivec2 position;
	bool active;
	bool colliding;

	float angle;
	int neighbour;
} food_object;

static food_object food_list[MAX_FOOD_COUNT];
static int food_count = 0;

static int active_food_count = 0;
static CAT_ivec2 active_food_centroid;

static void refresh_food_states()
{
	// Active food mask, count, and centroid
	active_food_count = 0;
	active_food_centroid = (CAT_ivec2){0, 0};
	for (int i = 0; i < food_count; i++)
	{
		food_list[i].active =
			food_list[i].position.x > table_rect.min.x &&
			food_list[i].position.x < table_rect.max.x &&
			food_list[i].position.y > table_rect.min.y &&
			food_list[i].position.y < table_rect.max.y;

		if (food_list[i].active)
		{
			active_food_count += 1;
			active_food_centroid = CAT_ivec2_add(active_food_centroid, food_list[i].position);
		}
	}
	active_food_centroid = CAT_ivec2_div(active_food_centroid, active_food_count);

	// Collision mask
	for (int i = 0; i < food_count; i++)
		food_list[i].colliding = false;
	for (int i = 0; i < food_count; i++)
	{
		if (!food_list[i].active)
			continue;

		for (int j = i + 1; j < food_count; j++)
		{
			if (!food_list[j].active)
				continue;

			int collision_distance = (FOOD_COLLISION_R * 0.9f) * 2;
			if (CAT_ivec2_dist2(food_list[i].position, food_list[j].position) < collision_distance * collision_distance)
			{
				food_list[i].colliding = food_list[j].colliding = true;
				break;
			}
		}
	}

	// Angles
	for (int i = 0; i < food_count; i++)
	{
		if (!food_list[i].active)
			continue;

		CAT_ivec2 spoke = CAT_ivec2_sub(food_list[i].position, active_food_centroid);
		float angle = atan2(-spoke.y, spoke.x);
		if (angle < 0)
			angle += M_PI * 2;
		food_list[i].angle = angle;
	}

	// CCW Sorting and neighbour map
	if (active_food_count > 1)
	{
		// Clear current values
		for (int i = 0; i < food_count; i++)
			food_list[i].neighbour = -1;

		// Identify the wraparound point:
		// The food with the largest CCW angle should point that with the smallest
		int max_angle_idx = -1;
		int min_angle_idx = -1;
		float max_angle = -INFINITY;
		float min_angle = INFINITY;
		for (int i = 0; i < food_count; i++)
		{
			if (!food_list[i].active)
				continue;

			float angle = food_list[i].angle;
			if (angle > max_angle)
			{
				max_angle_idx = i;
				max_angle = angle;
			}
			if (angle < min_angle)
			{
				min_angle_idx = i;
				min_angle = angle;
			}
		}
		food_list[max_angle_idx].neighbour = min_angle_idx;

		// Identify all other neighbour relations
		for (int i = 0; i < food_count; i++)
		{
			if (!food_list[i].active)
				continue;

			// Skip the wraparound point as our usual logic won't work for it
			if (i == max_angle_idx)
				continue;

			float this_angle = food_list[i].angle;
			for (int j = 0; j < food_count; j++)
			{
				if (j == i || !food_list[j].active)
					continue;

				int best_neighbour = food_list[i].neighbour;
				float best_angle =
					best_neighbour == -1 ? INFINITY : food_list[best_neighbour].angle;

				float candidate_angle = food_list[j].angle;
				if (candidate_angle > this_angle && candidate_angle < best_angle)
					food_list[i].neighbour = j;
			}
		}
	}
}

static float get_spawn_rect_fill(int rect_idx)
{
	CAT_rect spawn_rect = spawn_rects[rect_idx];
	int spawn_w = spawn_rect.max.x - spawn_rect.min.x;
	int spawn_h = spawn_rect.max.y - spawn_rect.min.y;
	int spawn_a = spawn_w * spawn_h;

	int overlap_a = 0;
	for (int i = 0; i < food_count; i++)
	{
		CAT_rect food_rect = CAT_rect_center(
			food_list[i].position.x, food_list[i].position.y,
			FOOD_COLLISION_W, FOOD_COLLISION_H);
		CAT_rect overlap = CAT_rect_overlap(spawn_rect, food_rect);
		int w = overlap.max.x - overlap.min.x;
		int h = overlap.max.y - overlap.min.y;
		w = w > 0 ? w : 0;
		h = h > 0 ? h : 0;
		overlap_a += w * h;
	}

	return (float)overlap_a / (float)spawn_a;
}

static CAT_ivec2 get_spawn_position()
{
	int free_idx = -1;
	float min_fill = 2.0f;
	for (int i = 0; i < 5; i++)
	{
		float fill = get_spawn_rect_fill(i);
		if (fill < min_fill)
		{
			free_idx = i;
			min_fill = fill;
		}
	}

	CAT_rect rect = spawn_rects[free_idx];
	float x = rect.min.x;
	float y = rect.min.y;
	float w = rect.max.x - x;
	float h = rect.max.y - y;
	return (CAT_ivec2){x + w / 2, y + h / 2};
}

static int food_spawn(int pool_idx)
{
	if (food_count >= MAX_FOOD_COUNT)
		return -1;
	if(pool_idx == -1)
		return -1;

	int list_idx = food_count;
	food_list[list_idx] = (food_object){
		.pool_idx = pool_idx,

		.position = get_spawn_position(),
		.active = false,
		.colliding = false,

		.angle = 0,
		.neighbour = -1};
	food_count += 1;

	refresh_food_states();
	return list_idx;
}

static void food_despawn(int list_idx)
{
	if (list_idx < 0 || list_idx >= MAX_FOOD_COUNT)
		return;
	food_count -= 1;
	for (int i = list_idx; i < food_count; i++)
	{
		food_list[i] = food_list[i + 1];
	}

	refresh_food_states();
}

static CAT_item *food_lookup(int list_idx)
{
	return CAT_get_item(food_pool.data[food_list[list_idx].pool_idx]);
}

static void init_food_list()
{
	food_count = 0;
	/*for (int i = 0; i < min(food_pool.length, MAX_FOOD_COUNT); i++)
		food_spawn(i);*/
}

static struct
{
	float variety;
	float propriety;
	float ichiju_sansai;
	float spacing;
	float evenness;
	float aggregate;
	int grade;
} score_object = {0};

static struct
{
	const char *message;
	enum
	{
		BASIC,
		BAD,
		GOOD,
		SPECIAL
	} severity;
} note_list[12];
static int note_count = 0;

static int add_note(const char *message, int severity)
{
	if (note_count >= 12)
		return -1;
	int idx = note_count;
	note_list[idx].message = message;
	note_list[idx].severity = severity;
	note_count += 1;
	return idx;
}

int centerpiece_idx = -1;

static float score_variety()
{
	if (active_food_count == 0)
		return 0.0f;

	// Ideally a meal has:
	// At least some veg (3pt)
	// At least as much veg as meat (1pt)
	// No more than 1 starch (1pt)
	// All major groups present (1pt)

	float point_total = 5.0f;
	float points = point_total;

	int veg_count = 0;
	int starch_count = 0;
	int meat_count = 0;
	int dairy_count = 0;

	for (int i = 0; i < food_count; i++)
	{
		if (!food_list[i].active)
			continue;

		CAT_item *food = food_lookup(i);
		CAT_food_group group = food->food_group;
		switch (group)
		{
		case CAT_FOOD_GROUP_VEG:
			veg_count += 1;
			break;
		case CAT_FOOD_GROUP_STARCH:
			starch_count += 1;
			break;
		case CAT_FOOD_GROUP_MEAT:
			meat_count += 1;
			break;
		case CAT_FOOD_GROUP_DAIRY:
			dairy_count += 1;
			break;
		default:
			break;
		}
	}

	if (veg_count < 1)
	{
		points -= 2;
		add_note("Needs more vegetables", BAD);
	}
	if (veg_count < meat_count)
	{
		points -= 1;
		add_note("Heavy on the meat", BASIC);
	}
	if (veg_count > 2)
	{
		add_note("Lots of veggies!", GOOD);
	}

	if (starch_count > 1)
	{
		points -= 1;
		add_note("One starch will do", BASIC);
	}

	if (
		veg_count == 0 ||
		meat_count == 0 ||
		starch_count == 0 ||
		dairy_count == 0)
	{
		points -= 1;
		add_note("Could use more variety", BASIC);
	}
	else
	{
		add_note("Highly varied", GOOD);
	}

	return clamp(points / point_total, 0, 1);
}

static float score_propriety()
{
	if (active_food_count == 0)
		return 0.0f;

	// Ideally a meal has:
	// Exactly one main (2pt)
	// Exactly one staple (1pt)
	// No treats if no sides are present (1pt)
	// At most one treat if sides are present (1pt)
	// No vices (As many demerits as vices)

	float point_total = 5.0f;
	float points = point_total;

	int staple_count = 0;
	int main_count = 0;
	int treat_count = 0;
	int vice_count = 0;
	int side_count = 0;

	for (int i = 0; i < food_count; i++)
	{
		if (!food_list[i].active)
			continue;

		CAT_item *food = food_lookup(i);
		switch (food->food_role)
		{
		case CAT_FOOD_ROLE_STAPLE:
			staple_count += 1;
			break;
		case CAT_FOOD_ROLE_MAIN:
			main_count += 1;
			break;
		case CAT_FOOD_ROLE_TREAT:
			treat_count += 1;
			break;
		case CAT_FOOD_ROLE_VICE:
			vice_count += 1;
			break;
		case CAT_FOOD_ROLE_SIDE:
		case CAT_FOOD_ROLE_SOUP:
			side_count += 1;
			break;
		default:
			break;
		}
	}

	if (main_count == 0)
	{
		points -= 3;
		add_note("No main course?!", BAD);
	}
	else if (main_count > 1)
	{
		points -= 2;
		add_note("One main is plenty", BAD);
	}

	if (staple_count != 1)
	{
		points -= 1;
		add_note("Lacking a staple", BASIC);
	}

	if (side_count == 0 && treat_count > 0)
	{
		points -= 1;
		add_note("Prioritize the basics", BASIC);
	}
	else if (side_count > 0 && treat_count > 1)
	{
		points -= 1;
		add_note("Heavy on the treats", BASIC);
	}
	if (treat_count > 2)
	{
		add_note("Very indulgent", BAD);
	}

	points -= vice_count;
	if (vice_count > 1)
	{
		add_note("Riddled with vice", BAD);
	}
	else if (vice_count == 1)
	{
		add_note("Vice is best avoided", BASIC);
	}
	else if (vice_count == 0 && treat_count == 0)
	{
		add_note("Impressive restraint", GOOD);
	}

	return clamp(points / point_total, 0, 1.0f);
}

static float score_ichiju_sansai()
{
	if (active_food_count == 0)
		return 0.0f;

	float staple = 0;
	float soup = 0;
	float main = 0;
	float sides = 0;

	for (int i = 0; i < food_count; i++)
	{
		if (!food_list[i].active)
			continue;

		CAT_item *food = food_lookup(i);
		if (food->food_role == CAT_FOOD_ROLE_SOUP && soup < 1)
			soup += 1;
		else if (food->food_role == CAT_FOOD_ROLE_SIDE && sides < 2)
			sides += 1;
		else if (food->food_role == CAT_FOOD_ROLE_MAIN && main < 1)
			main += 1;
		else if (food->food_role == CAT_FOOD_ROLE_STAPLE && staple < 1)
			staple += 1;
	}

	float ichisan = (staple + soup + main + sides) / 5.0f;
	if (ichisan == 1.0f)
		add_note("Ichi-ju san-sai!", SPECIAL);

	return ichisan;
}

static float score_spacing()
{
	if (active_food_count == 0)
		return 0.0f;
	if (active_food_count < 3)
		return 0.5f;

	int collision_count = 0;
	for (int i = 0; i < 5; i++)
	{
		if (food_list[i].colliding)
			collision_count += 1;
	}

	if (collision_count >= 4)
		add_note("Terribly crowded", BAD);
	else if (collision_count >= 2)
		add_note("A little crowded", BASIC);

	return active_food_count > 0 ? 1.0f - (float)collision_count / (float)active_food_count : 0;
}

static float score_evenness()
{
	if (active_food_count == 0)
		return 0.0f;
	if (active_food_count < 3)
		return 0.5f;

	centerpiece_idx = -1;
	for (int i = 0; i < food_count; i++)
	{
		if (!food_list[i].active)
			continue;

		if (CAT_ivec2_dist2(food_list[i].position, active_food_centroid) < CENTERPIECE_RADIUS * CENTERPIECE_RADIUS)
		{
			centerpiece_idx = i;
			break;
		}
	}
	int surrounding_count = centerpiece_idx == -1 ? active_food_count : active_food_count - 1;

	float spokes[5];
	float edges[5];

	float spoke_mean = 0.0f;
	float edge_mean = 0.0f;
	for (int i = 0; i < food_count; i++)
	{
		if (!food_list[i].active || i == centerpiece_idx)
			continue;

		CAT_ivec2 this_pos = food_list[i].position;
		spokes[i] = sqrt(CAT_ivec2_dist2(active_food_centroid, this_pos));
		spoke_mean += spokes[i];

		CAT_ivec2 neighbour_pos = food_list[food_list[i].neighbour].position;
		edges[i] = sqrt(CAT_ivec2_dist2(neighbour_pos, this_pos));
		edge_mean += edges[i];
	}
	spoke_mean /= (float)surrounding_count;
	edge_mean /= (float)surrounding_count;

	float spoke_stddev = 0.0f;
	float edge_stddev = 0.0f;
	for (int i = 0; i < food_count; i++)
	{
		if (!food_list[i].active || i == centerpiece_idx)
			continue;

		spoke_stddev += (spokes[i] - spoke_mean) * (spokes[i] - spoke_mean);
		edge_stddev += (edges[i] - edge_mean) * (edges[i] - edge_mean);
	}
	spoke_stddev = sqrt(spoke_stddev / (float)(surrounding_count - 1));
	edge_stddev = sqrt(edge_stddev / (float)(surrounding_count - 1));

	float evenness = CAT_ease_in_sine(
		1.0f -
		((spoke_stddev / spoke_mean) +
		 (edge_stddev / edge_mean)) *
			0.5f);

	if (surrounding_count <= 2)
		return 1.0f;
	else
	{
		if (evenness > 0.9f)
			add_note("Impeccably spaced", SPECIAL);
		if (evenness > 0.65f)
			add_note("Pleasingly spaced", GOOD);
		else if (evenness > 0.45f)
			add_note("Placement a bit awkward", BASIC);
		else
			add_note("Nonsensical placement", BAD);

		return evenness;
	}
}

static void refresh_scores()
{
	note_count = 0;
	if (active_food_count == 0)
		add_note("That was nothing!", BAD);

	score_object.variety = score_variety();
	score_object.propriety = score_propriety();
	score_object.ichiju_sansai = score_ichiju_sansai();
	score_object.spacing = score_spacing();
	score_object.evenness = score_evenness();

	score_object.aggregate =
		(score_object.variety * 2 +
		 score_object.propriety * 2 +
		 score_object.ichiju_sansai * 0.5f +
		 score_object.spacing +
		 score_object.evenness) /
		6.5f;
	score_object.aggregate = clamp(score_object.aggregate, 0, 1.0f);
	score_object.aggregate = CAT_ease_in_sine(score_object.aggregate);

	score_object.grade = round(score_object.aggregate * 6.0f);
}

static void init_scores()
{
	memset(&score_object, 0, sizeof(score_object));
}

static int pick_idx = -1;
static CAT_ivec2 pick_delta;

static bool show_feedback = false;
static float feedback_timer = 0;
static bool show_gizmos = false;
static bool show_debug_text = false;

void MS_feed_arrange(CAT_machine_signal signal)
{
	switch (signal)
	{
	case CAT_MACHINE_SIGNAL_ENTER:
		CAT_set_render_callback(render_arrange);
		pick_idx = -1;
		feedback_timer = 0;
		show_feedback = false;
		break;

	case CAT_MACHINE_SIGNAL_TICK:
		if (CAT_gui_popup_is_open())
			break;
		if (CAT_input_pressed(CAT_BUTTON_A))
			CAT_gui_open_popup("Submit this meal?\nFood items on table\nwill be consumed!\n");
		if (CAT_gui_consume_popup())
		{
			CAT_machine_transition(MS_feed_summary);
			break;
		}

		if (CAT_input_pressed(CAT_BUTTON_B))
			CAT_machine_transition(CAT_MS_room);
		if (CAT_input_pressed(CAT_BUTTON_SELECT) && food_pool.length > 0)
			CAT_machine_transition(MS_feed_select);

		if (CAT_input_touch_rect(85, 240, 64, 64) && !show_feedback)
		{
			show_feedback = true;
			feedback_timer = 0;
		}

		if (CAT_input_touch_down())
		{
			bool picked = false;
			for (int i = 0; i < food_count; i++)
			{
				if (CAT_input_cursor_in_circle(food_list[i].position.x, food_list[i].position.y, FOOD_COLLISION_R))
				{
					food_object temp = food_list[0];
					food_list[0] = food_list[i];
					food_list[i] = temp;

					pick_idx = 0;
					pick_delta = (CAT_ivec2){
						food_list[0].position.x - CAT_input_cursor().x,
						food_list[0].position.y - CAT_input_cursor().y,
					};

					picked = true;
					break;
				}
			}
			
			if(!picked && CAT_input_touch_rect(COUNTER_X, COUNTER_Y, COUNTER_W, COUNTER_H) && food_pool.length > 0)
				CAT_machine_transition(MS_feed_select);
		}
		else if (CAT_input_touch_up())
		{
			refresh_food_states();
			refresh_scores();

			pick_idx = -1;
		}
		else if (pick_idx != -1)
		{
			food_list[pick_idx].position = (CAT_ivec2){
				CAT_input_cursor().x + pick_delta.x,
				CAT_input_cursor().y + pick_delta.y};
		}

		if(show_feedback)
		{
			if(feedback_timer >= 1)
				show_feedback = false;
			feedback_timer += CAT_get_delta_time_s();
		}
		break;

	case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}

static void render_feedback()
{
	CAT_set_sprite_flags(CAT_DRAW_FLAG_BOTTOM);
	const CAT_sprite *sprite = &pet_feed_neutral_sprite;
	int x = 240 - 96;
	int y = 320;
	if (score_object.grade == 1 || active_food_count == 0)
	{
		sprite = &pet_feed_very_bad_sprite;
	}
	else if (score_object.grade > 5 || score_object.grade > (active_food_count + 1))
	{
		sprite = &pet_feed_good_sprite;
	}
	else if (score_object.grade < active_food_count)
	{
		sprite = &pet_feed_bad_sprite;
	}
	CAT_draw_sprite_raw(sprite, 0, x, y);
}

static void render_arrange()
{
	for(int y = 64; y < 320; y+=16)
	{
		for(int x = 0; x < 240; x+=16)
		{
			CAT_draw_tile(&floor_basic_tile_sprite, 2, x, y);
		}
	}
	
	int counter_x = counter_rect.min.x;
	int counter_y = counter_rect.min.y;
	int counter_w = counter_rect.max.x - counter_x;
	int counter_h = counter_rect.max.y - counter_y;
	CAT_draw_background(&feed_upper_tray_sprite, 0, 0);

	if (food_pool.length <= 0)
	{
		CAT_set_text_scale(2);
		CAT_set_text_colour(CAT_WHITE);
		CAT_draw_textf(counter_x + 68, counter_y + 6, "Out of");
		CAT_set_text_scale(2);
		CAT_set_text_colour(CAT_WHITE);
		CAT_draw_textf(counter_x + 34, counter_y + 32, "food items!");
	}
	else if(food_count <= 0)
	{
		CAT_set_text_mask(counter_x+8, -1, counter_x+counter_w-8, -1);
		CAT_set_text_colour(CAT_WHITE);
		CAT_set_text_flags(CAT_TEXT_FLAG_WRAP | CAT_TEXT_FLAG_CENTER);
		CAT_draw_textf(counter_x+counter_w/2, counter_y+counter_h/2-18, "Press [SELECT] or touch here to select food items.\n");
	}
	else if(active_food_count == food_count)
	{
		CAT_set_text_mask(counter_x+8, -1, counter_x+counter_w-8, -1);
		CAT_set_text_colour(CAT_WHITE);
		CAT_set_text_flags(CAT_TEXT_FLAG_WRAP | CAT_TEXT_FLAG_CENTER);
		CAT_draw_textf(counter_x+counter_w/2, counter_y+counter_h/2-18, "Press [A] to submit your meal.\n");
	}

	int table_x = table_rect.min.x;	
	int table_y = table_rect.min.y;
	int table_w = table_rect.max.x - table_x;
	int table_h = table_rect.max.y - table_y;
	int center_x = table_x + table_w / 2;
	int center_y = table_y + table_h / 2;
	CAT_set_sprite_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y);
	CAT_draw_sprite_raw(&feed_table_sprite, 0, center_x, center_y + 16);
	CAT_set_sprite_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y);
	CAT_draw_sprite_raw(&feed_tablecloth_sprite, 0, center_x, center_y + 16);
	CAT_set_sprite_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y);
	CAT_draw_sprite_raw(&feed_tray_sprite, 0, center_x, center_y);

	CAT_draw_sprite_raw(&feed_flower_sprite, 0, table_x - 36, table_y - 48);
	CAT_draw_sprite_raw(&feed_chopsticks_sprite, 0, table_x + table_w + 4, table_y);

	if (show_gizmos)
	{
		CAT_strokeberry(counter_x, counter_y, counter_w, counter_h, CAT_RED);
		for (int i = 0; i < 5; i++)
		{
			CAT_rect spawn_rect = spawn_rects[i];
			int spawn_x = spawn_rect.min.x;
			int spawn_y = spawn_rect.min.y;
			int spawn_w = spawn_rect.max.x - spawn_x;
			int spawn_h = spawn_rect.max.y - spawn_y;
			CAT_strokeberry(spawn_x, spawn_y, spawn_w, spawn_h, CAT_WHITE);
		}

		CAT_strokeberry(table_x, table_y, table_w, table_h, CAT_BLUE);
	}

	for (int i = food_count - 1; i >= 0; i--)
	{
		CAT_item *food = food_lookup(i);
		CAT_ivec2 food_pos = food_list[i].position;
		CAT_rect food_rect = CAT_rect_center(food_pos.x, food_pos.y, FOOD_COLLISION_W, FOOD_COLLISION_H);
		CAT_set_sprite_scale(2);
		CAT_set_sprite_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_BOTTOM);
		CAT_draw_sprite(food->sprite, 0, food_pos.x, food_rect.max.y);

		if (show_gizmos)
		{
			CAT_circberry(food_list[i].position.x, food_list[i].position.y, FOOD_COLLISION_R, food_list[i].colliding ? CAT_RED : CAT_WHITE);

			if (food_list[i].active)
			{
				CAT_RGB888 red = CAT_RGB24(255, 0, 0);
				CAT_RGB888 green = CAT_RGB24(0, 255, 0);
				uint16_t evenness_colour = CAT_RGB24216(CAT_RGB24_lerp(red, green, score_object.evenness));

				CAT_lineberry(food_list[i].position.x, food_list[i].position.y, active_food_centroid.x, active_food_centroid.y, evenness_colour);

				int j = food_list[i].neighbour;
				if (j != -1 && food_list[j].active)
					CAT_lineberry(food_list[i].position.x, food_list[i].position.y, food_list[j].position.x, food_list[j].position.y, evenness_colour);
			}
		}
		if (show_debug_text)
		{
			if (food_list[i].active)
			{
				CAT_set_text_colour(CAT_WHITE);
				CAT_draw_textf
				(
					food_list[i].position.x - FOOD_COLLISION_W / 2,
					food_list[i].position.y - FOOD_COLLISION_H / 2 - 14,
					"%d: %0.2f", i, food_list[i].angle
				);
			}
		}
	}

	if (show_gizmos)
	{
		if (active_food_count > 0)
			CAT_circberry(active_food_centroid.x, active_food_centroid.y, CENTERPIECE_RADIUS, centerpiece_idx == -1 ? CAT_GREEN : CAT_RED);
	}

	CAT_set_sprite_flags(CAT_DRAW_FLAG_BOTTOM | CAT_DRAW_FLAG_CENTER_X);
	if (show_feedback)
	{
		CAT_set_sprite_colour(RGB8882565(64, 64, 64));
		CAT_draw_sprite_raw(&pet_feed_back_sprite, -1, 120, 320);
	}
	else
	{
		CAT_draw_sprite_raw(&pet_feed_back_sprite, -1, 120, 320);
	}
	if (show_feedback)
		render_feedback();

	if (show_debug_text)
	{
		CAT_gui_printf(CAT_WHITE, "variety: %0.2f", score_object.variety);
		CAT_gui_printf(CAT_WHITE, "propriety: %0.2f", score_object.propriety);
		CAT_gui_printf(CAT_WHITE, "ichisan: %0.2f", score_object.ichiju_sansai);
		CAT_gui_printf(CAT_WHITE, "spacing: %0.2f", score_object.spacing);
		CAT_gui_printf(CAT_WHITE, "evenness: %0.2f", score_object.evenness);
		CAT_gui_printf(CAT_WHITE, "aggregate: %0.2f", score_object.aggregate);
		CAT_gui_printf(CAT_WHITE, "level: %d", score_object.grade);
	}
}

static const char *group_strings[] =
{
	"VEG / FRUIT",
	"STARCH",
	"MEAT",
	"DAIRY",
	"MISCELLANY"
};

static const char *role_strings[] =
{
	"STAPLE",
	"MAIN",
	"SIDE",
	"SOUP",
	"DRINK",
	"TREAT",
	"VICE"
};

static int scroll_y_anchor = 0;
static int scroll_y_delta = 0;
static bool scrolling = false;

static int last_clicked_idx = -1;

static float inspect_timer = 0;
static int inspect_idx = -1;

static int get_hovered_idx()
{
	int x = SELECT_GRID_MARGIN;
	int y = SELECT_GRID_MARGIN + scroll_y_delta;
	int idx = 0;

	while (idx < food_pool.length && idx < food_pool.length)
	{
		for (int col = 0; col < 3; col++)
		{
			if (CAT_input_cursor_in_rect(x, y, 64, 64))
				return idx;

			x += 64 + 12;
			idx += 1;
			if (idx >= food_pool.length)
				break;
		}
		x = SELECT_GRID_MARGIN;
		y += 64 + SELECT_GRID_MARGIN;
	}

	return -1;
}

static int get_min_scroll_y()
{
	return -SELECT_GRID_MARGIN;
}

static int get_max_scroll_y()
{
	int rows = food_pool.length / 3 + (food_pool.length % 3 != 0);
	int pool_size = (64 + SELECT_GRID_MARGIN) * rows + SELECT_GRID_MARGIN - CAT_LCD_SCREEN_H;
	return pool_size > 0 ? pool_size : SELECT_GRID_MARGIN;
}

static void MS_feed_select(CAT_machine_signal signal)
{
	switch (signal)
	{
	case CAT_MACHINE_SIGNAL_ENTER:
		CAT_set_render_callback(render_select);
		scroll_y_anchor = 0;
		scroll_y_delta = 0;
		last_clicked_idx = -1;
		inspect_timer = 0;
		inspect_idx = -1;
		break;

	case CAT_MACHINE_SIGNAL_TICK:
		if (CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_SELECT))
			CAT_machine_transition(MS_feed_arrange);

		if (CAT_input_touching())
		{
			// We always want to know this
			int hovered_idx = get_hovered_idx();

			// If this is the click frame, register any clicked box
			if (CAT_input_touch_down())
			{
				scroll_y_anchor = input.touch.y;
				last_clicked_idx = hovered_idx;
			}

			// Detect if this is a scroll action and quit early if so
			int scroll_y = input.touch.y;
			int scroll_dy = scroll_y - scroll_y_anchor;
			if (abs(scroll_dy) > 4)
			{
				scroll_y_anchor = scroll_y;
				scroll_y_delta += scroll_dy;
				scroll_y_delta = -clamp(-scroll_y_delta, get_min_scroll_y(), get_max_scroll_y());

				scrolling = true;
				last_clicked_idx = -1;
				inspect_timer = 0;
				return;
			}

			// If we have left the last clicked box, cancel any inspection
			if (hovered_idx != last_clicked_idx)
			{
				last_clicked_idx = -1;
				inspect_timer = 0;
			}
			// Otherwise continue to inspection logic
			else if (!scrolling && last_clicked_idx != -1)
			{
				if (input.touch_time >= 0.5f)
				{
					if(inspect_timer >= 1.0f)
					{
						inspect_idx = last_clicked_idx;
						CAT_machine_transition(MS_feed_inspect);
						inspect_timer = 0;
					}
					inspect_timer += CAT_get_delta_time_s();
				}
			}
		}
		else if (CAT_input_touch_up())
		{
			// Only register de/selection if not scrolling
			if (!scrolling)
			{
				int list_idx = -1;
				for (int i = 0; i < food_count; i++)
				{
					if (food_list[i].pool_idx == last_clicked_idx)
					{
						list_idx = i;
						break;
					}
				}

				if (list_idx == -1)
				{
					if (food_count == 5)
						food_despawn(4);
					if(last_clicked_idx != -1)
						food_spawn(last_clicked_idx);
				}
				else
					food_despawn(list_idx);
			}

			scrolling = false;
			last_clicked_idx = -1;
			inspect_timer = 0;
		}

		if (CAT_input_held(CAT_BUTTON_UP, 0))
			scroll_y_delta += 32;
		if (CAT_input_held(CAT_BUTTON_DOWN, 0))
			scroll_y_delta -= 32;
		scroll_y_delta = -clamp(-scroll_y_delta, get_min_scroll_y(), get_max_scroll_y());
		break;

	case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}

static void render_select()
{
	CAT_frameberry(0xbdb4);

	int x = SELECT_GRID_MARGIN;
	int y = SELECT_GRID_MARGIN + scroll_y_delta;
	int idx = 0;
	while (idx < food_pool.length)
	{
		for (int col = 0; col < 3; col++)
		{
			CAT_draw_sprite(&ui_item_frame_bg_sprite, 0, x, y);

			CAT_item *food = CAT_get_item(food_pool.data[idx]);
			CAT_set_sprite_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y);
			CAT_set_sprite_scale(2);
			CAT_draw_sprite(food->sprite, 0, x + 32, y + 32);

			for (int i = 0; i < food_count; i++)
			{
				if (food_list[i].pool_idx == idx)
				{
					CAT_draw_sprite(&ui_item_frame_fg_sprite, 0, x, y);
					break;
				}
			}

			if (idx == last_clicked_idx && inspect_timer >= 0.05f)
				CAT_annulusberry(input.touch.x, input.touch.y, 24, 18, RGB8882565(255 - 0, 255 - 141, 255 - 141), inspect_timer + 0.15f, 0);

			idx += 1;
			x += 64 + 12;
			if (idx >= food_pool.length)
				break;
		}

		x = SELECT_GRID_MARGIN;
		y += 64 + SELECT_GRID_MARGIN;
	}

	if (abs(-scroll_y_delta - get_max_scroll_y()) >= 64)
	{
		CAT_draw_sprite(&ui_down_arrow_sprite, -1, 240 - 32, 320 - 24);
	}
}

static void MS_feed_inspect(CAT_machine_signal signal)
{
	switch (signal)
	{
	case CAT_MACHINE_SIGNAL_ENTER:
		CAT_set_render_callback(render_inspect);
		break;

	case CAT_MACHINE_SIGNAL_TICK:
		if (CAT_input_pressed(CAT_BUTTON_B))
			CAT_machine_transition(MS_feed_select);
		else if (CAT_input_pressed(CAT_BUTTON_SELECT))
			CAT_machine_transition(MS_feed_arrange);

		if (CAT_input_pressed(CAT_BUTTON_RIGHT))
			inspect_idx += 1;
		if (CAT_input_pressed(CAT_BUTTON_LEFT))
			inspect_idx -= 1;
		inspect_idx = (inspect_idx + food_pool.length) % food_pool.length;
		break;

	case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}

static void render_inspect()
{
	CAT_frameberry(RGB8882565(142, 171, 174));

	CAT_item *inspectee = CAT_get_item(food_pool.data[inspect_idx]);
	if (inspectee == NULL)
		return;

	CAT_set_text_scale(2);
	CAT_set_text_colour(CAT_WHITE);
	CAT_draw_text(8, 8, inspectee->name);
	CAT_set_text_colour(CAT_WHITE);
	CAT_draw_textf(8, 8 + 28, "Group: %s", group_strings[inspectee->food_group]);
	CAT_set_text_colour(CAT_WHITE);
	CAT_draw_textf(8, 8 + 28 + 16, "Role: %s", role_strings[inspectee->food_role]);
	CAT_set_text_colour(CAT_WHITE);
	CAT_draw_text(8, 8 + 28 + 16 + 16, inspectee->text);

	CAT_set_sprite_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y);
	CAT_set_sprite_scale(6);
	CAT_draw_sprite(inspectee->sprite, 0, 120, 160);
}

static enum {
	PERFORMANCE,
	VARIETY,
	PROPRIETY,
	LAYOUT,
	NOTES,
	REWARDS,
	SUMMARY_PAGE_MAX
} summary_page = PERFORMANCE;

static const int xp_rewards[] =
{
	0, 0,
	1, 3,
	5, 8,
	8, 12,
	12, 18,
	18, 25
};

static int xp_reward;

static void MS_feed_summary(CAT_machine_signal signal)
{
	switch (signal)
	{
	case CAT_MACHINE_SIGNAL_ENTER:
		CAT_set_render_callback(render_summary);
		summary_page = PERFORMANCE;
		xp_reward = CAT_rand_int(xp_rewards[score_object.grade*2+0], xp_rewards[score_object.grade*2+1]);
		break;

	case CAT_MACHINE_SIGNAL_TICK:
		if (CAT_input_pressed(CAT_BUTTON_A) || CAT_input_pressed(CAT_BUTTON_B))
			CAT_machine_transition(CAT_MS_room);

		// enum = (enum + ENUM_MAX) % ENUM_MAX doesn't work on embedded
		int summary_page_proxy = summary_page;
		if (CAT_input_pressed(CAT_BUTTON_RIGHT))
			summary_page_proxy += 1;
		if (CAT_input_pressed(CAT_BUTTON_LEFT))
			summary_page_proxy -= 1;
		summary_page = (summary_page_proxy + SUMMARY_PAGE_MAX) % SUMMARY_PAGE_MAX;
		break;

	case CAT_MACHINE_SIGNAL_EXIT:
			for (int i = 0; i < food_count; i++)
			{
				if (food_list[i].active)
					CAT_inventory_remove(food_pool.data[food_list[i].pool_idx], 1);
			}
			CAT_pet_change_vigour(score_object.grade);
			CAT_pet_change_XP(xp_reward);
		break;
	}
}

static uint16_t grade_colours[6] =
{
		0x8082, // F
		0x81e2, // C
		0xbae4, // B-
		0xd506, // B
		0xad49, // A-
		0x7d67	// A
};

static uint16_t severity_colours[4] =
{
		CAT_BLACK, // BASIC
		0xc983,	   // BAD
		0x4c07,	   // GOOD
		0x82B9,	   // SPECIAL
};

static CAT_ivec2 registration_errors[SUMMARY_PAGE_MAX];

static void init_registration_errors()
{
	for (int i = 0; i < SUMMARY_PAGE_MAX; i++)
	{
		registration_errors[i] = (CAT_ivec2)
		{
			CAT_rand_int(-12, 12),
			CAT_rand_int(-12, 12)
		};
	}
}

static int get_glyph_idx(int grade)
{
	if (grade == 2 || grade == 3)
		return 2;
	if (grade == 4 || grade == 5)
		return 3;
	else
		return grade;
}

static void render_plus_line(int x, int y, int w, float min, float base, float plus, float max)
{
	int overshoot = (base+plus) - max;
	if(overshoot > 0)
		plus -= overshoot;

	int start_x = x;
	int base_x = x + w * inv_lerp(base, min, max);
	int plus_x = x + w * inv_lerp(base+plus, min, max);
	int end_x = x + w;
	CAT_lineberry(start_x, y, base_x, y, 0xd506);
	CAT_lineberry(base_x, y, plus_x, y, 0x54a2);
	CAT_lineberry(plus_x, y, end_x, y, 0xc618);
}

static void render_summary()
{
	CAT_frameberry(CAT_PAPER_CREAM);

	const char *title = "N/A";
	int grade = 0;

	switch (summary_page)
	{
	case PERFORMANCE:
		title = "Performance";
		grade = score_object.grade;
		break;
	case VARIETY:
		title = "Variety";
		grade = round(score_object.variety * 5);
		break;
	case PROPRIETY:
		title = "Nutrition";
		grade = round(score_object.propriety * 5);
		break;
	case LAYOUT:
		title = "Layout";
		grade = round((score_object.spacing + score_object.evenness) * 0.5f * 5);
		break;
	case NOTES:
		title = "Notes";
		break;
	case REWARDS:
		title = "Rewards";
		break;
	default:
		break;
	}

	CAT_set_sprite_colour(RGB8882565(64, 64, 64));
	CAT_draw_sprite(&ui_left_arrow_sprite, -1, 8, 12);
	CAT_set_sprite_colour(RGB8882565(64, 64, 64));
	CAT_draw_sprite(&ui_right_arrow_sprite, -1, 240 - 13 - 8, 12);
	int title_len = strlen(title);
	int title_x = (CAT_LCD_SCREEN_W - 1 - title_len * 16) / 2;
	CAT_set_text_colour(CAT_BLACK);
	CAT_set_text_scale(2);
	CAT_draw_text(title_x, 12, title);

	if (summary_page == NOTES)
	{
		int cursor_y = 52;
		for (int i = 0; i < note_count; i++)
		{
			CAT_set_text_colour(severity_colours[note_list[i].severity]);
			CAT_draw_textf(12, cursor_y, "\1 %s", note_list[i].message);
			cursor_y += 18;
		}
		const char *signature = "- Inspector Reed";
		CAT_draw_text(240 - strlen(signature) * 8 - 12, cursor_y + 6, signature);
	}
	else if(summary_page == REWARDS)
	{
		int cursor_y = 52;

		CAT_set_text_colour(CAT_BLACK);
		CAT_draw_textf(12, cursor_y, "+ Vigour: %d", score_object.grade);
		cursor_y += 20;
		render_plus_line(12, cursor_y, CAT_LCD_SCREEN_W * 0.75, 0, pet.vigour, score_object.grade, 12);
		cursor_y += 16;

		CAT_set_text_colour(CAT_BLACK);
		CAT_draw_textf(12, cursor_y, "+ XP: %d", xp_reward);
		cursor_y += 20;
		render_plus_line(12, cursor_y, CAT_LCD_SCREEN_W * 0.75, 0, pet.xp, xp_reward, level_cutoffs[pet.level]);
		cursor_y += 16;
	}
	else
	{
		CAT_draw_cross_box(120-80, 160-100, 120+80, 160+100, CAT_GREY);

		CAT_set_sprite_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y);
		CAT_set_sprite_colour(grade_colours[grade]);
		CAT_draw_sprite(&ui_feed_stamp_base_sprite, grade == 2 || grade == 4, 120 + registration_errors[summary_page].x, 180 + registration_errors[summary_page].y);

		int glyph_idx = get_glyph_idx(grade);
		CAT_set_sprite_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y);
		CAT_set_sprite_colour(grade_colours[grade]);
		CAT_draw_sprite(&ui_feed_stamp_glyphs_sprite, glyph_idx, 120 + registration_errors[summary_page].x, 180 + registration_errors[summary_page].y);
	}
}

void CAT_MS_feed(CAT_machine_signal signal)
{
	switch (signal)
	{
	case CAT_MACHINE_SIGNAL_ENTER:
	{
		init_spawn_rects();
		init_item_id_pool();
		init_food_list();
		refresh_food_states();
		init_scores();
		init_registration_errors();
		refresh_scores();
	}
	break;

	case CAT_MACHINE_SIGNAL_TICK:
		CAT_machine_transition(MS_feed_arrange);
		break;

	case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}