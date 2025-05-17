#include "cat_actions.h"

#include "cat_math.h"
#include "cat_input.h"
#include "cat_render.h"
#include "sprite_assets.h"
#include "item_assets.h"
#include "cat_item.h"
#include "cat_bag.h"
#include "cowtools/cat_structures.h"
#include "cat_gui.h"
#include "cowtools/cat_curves.h"
#include "cat_pet.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void render_ring(int x, int y, int R, int r, uint16_t c, float t)
{
	for(int dy = -R; dy < R; dy++)
	{
		for(int dx = -R; dx < R; dx++)
		{
			if((dx*dx+dy*dy) <= R*R && (dx*dx+dy*dy) >= r*r)
			{
				float arc = atan2f((float) dy, (float) dx) + M_PI - (M_PI / 8);
				if(arc <= t * M_PI * 2)
					CAT_pixberry(x+dx, y+dy, c);
			}
		}
	}
}

void render_text(int x, int y, uint16_t c, int scale, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	char text[128];
	vsnprintf(text, 128, fmt, args);
	va_end(args);

	int draw_x = x;
	int draw_y = y;
	const char* g = text;
	while(*g != '\0')
	{
		char glyph = *g;
		if(glyph == ' ')
			draw_x += 5 * scale;
		else if(glyph == '\n')
		{
			draw_x = x;
			draw_y += 13 * scale;
		}
		else
		{
			CAT_push_draw_colour(c);
			CAT_push_draw_scale(scale);
			CAT_draw_sprite(&glyph_sprite, glyph, draw_x, draw_y);
			draw_x += 8 * scale;
		}

		g++;
	}
}

static enum
{
	SELECT,
	INSPECT,
	ARRANGE,
	SUMMARY,
} mode = ARRANGE;

static CAT_rect menu_rect =
{
	.min = {0, 0},
	.max = {240, 80}
};

static CAT_rect spawn_rects[5] =
{
	{{32, 8}, {32+48, 8+48}},
	{{64, 8}, {64+48, 8+48}},
	{{96, 8}, {96+48, 8+48}},
	{{128, 8}, {128+48, 8+48}},
	{{160, 8}, {160+48, 8+48}}
};

static CAT_rect table_rect =
{
	.min = {0, 96},
	.max = {240, 256}
};

static CAT_item_list food_pool;
static int idx_pool[CAT_ITEM_LIST_MAX_LENGTH];
static int food_idxs[5];
static CAT_rect food_rects[5];
static CAT_int_list food_idxs_l;

static bool food_active_mask[5];
static int food_active_count;
static CAT_vec2 food_centers[5];
static CAT_vec2 food_centroid;
static float food_angles[5];
static int food_neighbour_map[5];
static bool food_collision_mask[5];

static int touched = -1;
static int dx, dy;

static float group_score = 0.0f;
static float role_score = 0.0f;
static float ichisan_score = 0.0f;
static float spacing_score = 0.0f;
static float evenness_score = 0.0f;
static float aggregate_score = 0.0f;
static int level = 0;
static bool commit = false;
bool show_gizmos = false;
bool show_debug_text = false;

static const char* group_strings[] =
{
	"VEG / FRUIT",
	"STARCH",
	"MEAT",
	"DAIRY",
	"MISCELLANY"
};

static const char* role_strings[] =
{
	"STAPLE",
	"MAIN",
	"SIDE",
	"SOUP",
	"DRINK",
	"TREAT",
	"VICE"
};

static int select_grid_margin = 12;
static int scroll_last_touch_y = 0;
static int scroll_offset = 0;
static int last_selected = -1;
static bool scrolling = false;
static int inspect_timer_id = -1;
static int inspect_idx = -1;

static bool show_feedback = false;
static int feedback_timer_id = -1;

static enum
{
	PERFORMANCE,
	VARIETY,
	PROPRIETY,
	LAYOUT,
	SUMMARY_PAGE_MAX
} summary_page = PERFORMANCE;

static uint16_t grade_colours[6] =
{
	0x8082, // F
	0x81e2, // C
	0xbae4, // B-
	0xd506, // B
	0xad49, // A-
	0x7d67 // A
};

static CAT_vec2 stamp_jitters[SUMMARY_PAGE_MAX];

bool food_filter(int item_id)
{
	CAT_item* item = CAT_item_get(item_id);
	if(item == NULL)
		return false;
	return
	item->type == CAT_ITEM_TYPE_TOOL &&
	item->data.tool_data.type == CAT_TOOL_TYPE_FOOD;
}

void food_swap(int i, int j)
{
	int temp_id = food_idxs[i];
	food_idxs[i] = food_idxs[j];
	food_idxs[j] = temp_id;
	CAT_rect temp_rect = food_rects[i];
	food_rects[i] = food_rects[j];
	food_rects[j] = temp_rect;
	bool temp_active = food_active_mask[i];
	food_active_mask[i] = food_active_mask[j];
	food_active_mask[j] = temp_active;
	CAT_vec2 temp_center = food_centers[i];
	food_centers[i] = food_centers[j];
	food_centers[j] = temp_center;
	float temp_angle = food_angles[i];
	food_angles[i] = food_angles[j];
	food_angles[j] = temp_angle;
	int temp_neighbour = food_neighbour_map[i];
	food_neighbour_map[i] = food_neighbour_map[j];
	food_neighbour_map[j] = temp_neighbour;
	bool temp_collision = food_collision_mask[i];
	food_collision_mask[i] = food_collision_mask[j];
	food_collision_mask[j] = temp_collision;
}

CAT_item* food_get(int i)
{
	return CAT_item_get(food_pool.item_ids[food_idxs[i]]);
}

void food_delete(int i)
{
	CAT_ilist_delete(&food_idxs_l, i);
	for(int j = i; j < 5; j++)
	{
		food_rects[j] = food_rects[j+1];
		food_active_mask[j] = food_active_mask[j+1];
	}
}

CAT_rect rect_overlap(CAT_rect a, CAT_rect b)
{
	return (CAT_rect)
	{
		{ max(a.min.x, b.min.x), max(a.min.y, b.min.y) },
		{ min(a.max.x, b.max.x), min(a.max.y, b.max.y) }
	};
}

float spawn_rect_fill(int rect_idx)
{
	CAT_rect spawn_rect = spawn_rects[rect_idx];
	int spawn_w = spawn_rect.max.x - spawn_rect.min.x;
	int spawn_h = spawn_rect.max.y - spawn_rect.min.y;
	int spawn_a = spawn_w * spawn_h;

	int overlap_a = 0;
	for(int i = 0; i < food_idxs_l.length; i++)
	{
		CAT_rect food_rect = food_rects[i];
		CAT_rect overlap = rect_overlap(spawn_rect, food_rect);
		int w = overlap.max.x - overlap.min.x;
		int h = overlap.max.y - overlap.min.y;
		w = w > 0 ? w : 0;
		h = h > 0 ? h : 0;
		overlap_a += w * h;
	}

	return (float) overlap_a / (float) spawn_a;
}

void food_refresh()
{
	food_active_count = 0;
	food_centroid =(CAT_vec2) {0, 0};

	for(int i = 0; i < food_idxs_l.length; i++)
	{
		food_active_mask[i] = CAT_rect_contains(table_rect, food_rects[i]);
		if(!food_active_mask[i])
			continue;
		food_active_count += 1;

		food_centers[i] = (CAT_vec2)
		{
			(food_rects[i].min.x + food_rects[i].max.x) * 0.5f,
			(food_rects[i].min.y + food_rects[i].max.y) * 0.5f
		};
		food_centroid = CAT_vec2_add(food_centroid, food_centers[i]);
	}
	food_centroid = CAT_vec2_mul(food_centroid, 1.0f / (float) food_active_count);

	for(int i = 0; i < food_idxs_l.length; i++)
	{
		if(!food_active_mask[i])
			continue;

		CAT_vec2 spoke = CAT_vec2_sub(food_centers[i], food_centroid);
		float angle = atan2(-spoke.y, spoke.x);
		if(angle < 0)
			angle += M_PI * 2;
		food_angles[i] = angle;
	}

	for(int i = 0; i < food_idxs_l.length; i++)
		food_collision_mask[i] = false;
	for(int i = 0; i < food_idxs_l.length; i++)
	{
		if(!food_active_mask[i])
			continue;

		for(int j = i+1; j < food_idxs_l.length; j++)
		{
			if(!food_active_mask[j])
				continue;
			if(CAT_vec2_dist2(food_centers[i], food_centers[j]) < 44 * 44)
			{
				food_collision_mask[i] = food_collision_mask[j] = true;
				break;
			}
		}
	}
}

void food_spawn(int idx)
{
	int free_rect_idx = -1;
	float min_fill = 2.0f;
	for(int i = 0; i < 5; i++)
	{
		float fill = spawn_rect_fill(i);
		if(fill < min_fill)
		{
			free_rect_idx = i;
			min_fill = fill;
		}
	}

	CAT_rect spawn_rect = free_rect_idx != -1 ?
	spawn_rects[free_rect_idx] : (CAT_rect) {{96, 16}, {144, 64}};

	CAT_ilist_push(&food_idxs_l, idx);
	food_rects[food_idxs_l.length-1] = spawn_rect;

	food_refresh();
}

float group_diversity()
{
	if(food_active_count <= 0)
		return 0.0f;

	float veg = 0;
	float starch = 0;
	float meat = 0;
	float dairy = 0;

	for(int i = 0; i < food_idxs_l.length; i++)
	{
		if(!food_active_mask[i])
			continue;	
		CAT_item* food = food_get(i);
		if(food->data.tool_data.food_group == CAT_FOOD_GROUP_VEG && veg < 2)
			veg += 1;
		else if(food->data.tool_data.food_group == CAT_FOOD_GROUP_STARCH && starch < 1)
			starch += 1;
		else if(food->data.tool_data.food_group == CAT_FOOD_GROUP_MEAT && meat < 1)
			meat += 1;
		else if(food->data.tool_data.food_group == CAT_FOOD_GROUP_DAIRY && dairy < 1)
			dairy += 1;
	}

	return (veg + starch + meat + dairy) / 5.0f;
}

float role_propriety()
{
	if(food_active_count <= 0)
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

	for(int i = 0; i < food_idxs_l.length; i++)
	{
		if(!food_active_mask[i])
			continue;

		CAT_item* food = food_get(i);
		switch(food->data.tool_data.food_role)
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

	if(food_active_count > 1 && main_count != 1)
	{
		points -= 2;
	}
	else if(food_active_count == 1 && main_count != 1)
	{
		points -= 3;
	}
	if(staple_count != 1)
	{
		points -= 1;
	}
	if(side_count == 0 && treat_count > 0)
	{
		points -= 1;
	}
	else if(side_count > 0 && treat_count > 1)
	{
		points -= 1;
	}
	points -= vice_count;
	return clampf(points / point_total, 0, 1.0f);
}

float ichiju_sansai()
{
	if(food_active_count <= 0)
		return 0.0f;

	float staple = 0;
	float soup = 0;
	float main = 0;
	float sides = 0;

	for(int i = 0; i < food_idxs_l.length; i++)
	{
		if(!food_active_mask[i])
			continue;	
		CAT_item* food = food_get(i);
		if(food->data.tool_data.food_role == CAT_FOOD_ROLE_SOUP && soup < 1)
			soup += 1;
		else if(food->data.tool_data.food_role == CAT_FOOD_ROLE_SIDE && sides < 2)
			sides += 1;
		else if(food->data.tool_data.food_role == CAT_FOOD_ROLE_MAIN && main < 1)
			main += 1;
		else if(food->data.tool_data.food_role == CAT_FOOD_ROLE_STAPLE && staple < 1)
			staple += 1;
	}

	return (staple + soup + main + sides) / 5.0f;
}

float score_spacing()
{
	if(food_active_count <= 0)
		return 0.0f;
	if(food_active_count <= 1)
		return 0.5f;

	float collision_count = 0;
	for(int i = 0; i < 5; i++)
	{
		if(food_collision_mask[i])
			collision_count += 1;
	}

	return 
	food_active_count > 0 ?
	1.0f - collision_count / food_active_count :
	0;
}

void map_neighbours()
{
	if(food_active_count <= 1)
		return;

	for(int i = 0; i < food_idxs_l.length; i++)
	{
		food_neighbour_map[i] = -1;
	}

	int max_idx = -1;
	int min_idx = -1;
	float max_angle = -INFINITY;
	float min_angle = INFINITY;
	for(int i = 0; i < food_idxs_l.length; i++)
	{	
		if(!food_active_mask[i])
			continue;
		float angle = food_angles[i];
		if(angle > max_angle)
		{
			max_idx = i;
			max_angle = angle;
		}
		if(angle < min_angle)
		{
			min_idx = i;
			min_angle = angle;
		}
	}
	food_neighbour_map[max_idx] = min_idx;
	
	for(int i = 0; i < food_idxs_l.length; i++)
	{
		if(!food_active_mask[i])
			continue;
		if(i == max_idx)
			continue;

		float this_angle = food_angles[i];
		
		for(int j = 0; j < food_idxs_l.length; j++)
		{
			if(j == i || !food_active_mask[j])
				continue;

			int best_neighbour = food_neighbour_map[i];
			float best_angle = best_neighbour == -1 ?
			INFINITY : food_angles[best_neighbour];
			float candidate_angle = food_angles[j];
			if(candidate_angle > this_angle && candidate_angle < best_angle)
			{
				food_neighbour_map[i] = j;
			}
		}
	}
}

bool surrounding_centroid(int i)
{
	return
	food_active_mask[i] &&
	CAT_vec2_dist2(food_centers[i], food_centroid) >= 32*32;
}

float score_evenness()
{
	if(food_active_count == 0)
		return 0.0f;
	if(food_active_count <= 2)
		return 0.5f;

	float spokes[5];
	float edges[5];
	int surrounding_count = 0;

	float spoke_mean = 0.0f;
	float edge_mean = 0.0f;
	for(int i = 0; i < food_idxs_l.length; i++)
	{
		if(!surrounding_centroid(i))
			continue;
		surrounding_count += 1;

		spokes[i] = sqrt(CAT_vec2_dist2(food_centroid, food_centers[i]));
		spoke_mean += spokes[i];
		edges[i] = sqrt(CAT_vec2_dist2(food_centers[food_neighbour_map[i]], food_centers[i]));
		edge_mean += edges[i];
	}
	spoke_mean /= surrounding_count;
	edge_mean /= surrounding_count;

	float spoke_stddev = 0.0f;
	float edge_stddev = 0.0f;
	for(int i = 0; i < food_idxs_l.length; i++)
	{
		if(!surrounding_centroid(i))
			continue;

		spoke_stddev += (spokes[i] - spoke_mean) * (spokes[i] - spoke_mean);
		edge_stddev += (edges[i] - edge_mean) * (edges[i] - edge_mean);
	}
	spoke_stddev = sqrt(spoke_stddev / (surrounding_count-1));
	edge_stddev = sqrt(edge_stddev / (surrounding_count-1));

	if(surrounding_count <= 2)
		return 1.0f;
	else
	{
		return CAT_ease_in_sine
		(
			1.0f -
			((spoke_stddev / spoke_mean) +
			(edge_stddev / edge_mean)) * 0.5f
		);
	}
}

void score_refresh()
{
	group_score = group_diversity();
	role_score = role_propriety();
	ichisan_score = ichiju_sansai();
	spacing_score = score_spacing();
	evenness_score = score_evenness();
	aggregate_score = 
	(
		group_score * 2 +
		role_score * 2 +
		spacing_score +
		evenness_score +
		ichisan_score * 0.5f
	) / 6.5f;

	aggregate_score = clampf(aggregate_score, 0, 1.0f);
	aggregate_score = CAT_ease_in_sine(aggregate_score);

	level = round(aggregate_score * 6.0f);
}

int get_min_scroll_y()
{
	return -select_grid_margin;
}

int get_max_scroll_y()
{
	return ((food_pool.length / 3) + 3) * 64 + select_grid_margin - CAT_LCD_SCREEN_H;
}

int get_hovered()
{
	int x = select_grid_margin;
	int y = select_grid_margin + scroll_offset;
	int food_idx = 0;
	while(food_idx < food_pool.length)
	{
		for(int col = 0; col < 3; col++)
		{
			if
			(
				input.touch.x >= x && input.touch.x <= (x + 64) &&
				input.touch.y >= y && input.touch.y <= (y + 64)
			)
			{
				return food_idx;
				break;
			}

			food_idx += 1;
			x += 64 + 12;
			if(food_idx >= food_pool.length)
				break;
		}
		x = select_grid_margin;
		y += 64 + select_grid_margin;
	}
	return -1;
}

void select_grid_io()
{
	if(CAT_input_touching())
	{
		int currently_hovered = get_hovered();

		if(CAT_input_touch_down())
		{
			scroll_last_touch_y = input.touch.y;
			last_selected = currently_hovered;
			return;
		}

		int scroll_touch_y = input.touch.y;
		int scroll_dy = scroll_touch_y - scroll_last_touch_y;
		if(abs(scroll_dy) > 4)
		{
			scroll_offset += scroll_dy;
			scroll_last_touch_y = scroll_touch_y;
			scroll_offset = -clamp(-scroll_offset, get_min_scroll_y(), get_max_scroll_y());

			scrolling = true;
			CAT_timer_reset(inspect_timer_id);
			last_selected = -1;
			return;
		}
	
		if(currently_hovered != last_selected)
		{
			last_selected = -1;
			CAT_timer_reset(inspect_timer_id);
		}
		else if(!scrolling)
		{
			if(input.touch_time >= 0.5f)
			{
				if(CAT_timer_tick(inspect_timer_id))
				{
					CAT_timer_reset(inspect_timer_id);
					inspect_idx = last_selected;
					mode = INSPECT;
				}
			}
		}
	}
	else if(CAT_input_touch_up())
	{
		if(!scrolling)
		{
			int active_idx = CAT_ilist_find(&food_idxs_l, last_selected);
			if(active_idx != -1)
				food_delete(active_idx);
			else
			{
				if(food_idxs_l.length == 5)
					food_delete(4);
				food_spawn(last_selected);
			}
		}

		scrolling = false;
		CAT_timer_reset(inspect_timer_id);
		last_selected = -1;
	}
}

void render_select_grid()
{
	CAT_frameberry(0xbdb4);

	int x = select_grid_margin;
	int y = select_grid_margin + scroll_offset;
	int food_idx = 0;
	while(food_idx < food_pool.length)
	{
		for(int col = 0; col < 3; col++)
		{
			CAT_draw_sprite(&ui_item_frame_bg_sprite, 0, x, y);
			
			int food_id = food_pool.item_ids[food_idx];
			CAT_item* food = CAT_item_get(food_id);
			CAT_push_draw_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y);
			CAT_push_draw_scale(2);
			CAT_draw_sprite(food->sprite, 0, x + 32, y + 32);

			int active_idx = CAT_ilist_find(&food_idxs_l, food_idx);
			if(active_idx != -1)
				CAT_draw_sprite(&ui_item_frame_fg_sprite, 0, x, y);
			
			if(food_idx == last_selected && CAT_timer_progress(inspect_timer_id) >= 0.05f)
			{
				render_ring(input.touch.x, input.touch.y, 24, 18, RGB8882565(255-0, 255-141, 255-141), CAT_timer_progress(inspect_timer_id));
			}

			food_idx += 1;
			x += 64 + 12;
			if(food_idx >= food_pool.length)
				break;
		}
		x = select_grid_margin;
		y += 64 + select_grid_margin;
	}

	if(abs(-scroll_offset - get_max_scroll_y()) >= 64)
	{
		CAT_draw_sprite(&ui_down_arrow_sprite, -1, 240-32, 320-24);
	}
}

void render_inspector()
{
	CAT_frameberry(RGB8882565(142, 171, 174));

	CAT_item* inspectee = CAT_item_get(food_pool.item_ids[inspect_idx]);

	render_text(8, 8, CAT_WHITE, 2, inspectee->name);
	render_text(8, 8+28, CAT_WHITE, 1, "Group: %s", group_strings[inspectee->data.tool_data.food_group]);
	render_text(8, 8+28+16, CAT_WHITE, 1, "Role: %s", role_strings[inspectee->data.tool_data.food_role]);
	render_text(8, 8+28+16+16, CAT_WHITE, 1, inspectee->text);

	CAT_push_draw_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y);
	CAT_push_draw_scale(6);
	CAT_draw_sprite(inspectee->sprite, 0, 120, 160);
}

void render_feedback()
{
	CAT_push_draw_flags(CAT_DRAW_FLAG_BOTTOM);
	const CAT_sprite* sprite = &pet_feed_neutral_sprite;
	int x = 240-96; int y = 320;
	if(level == 1 || food_active_count == 0)
	{
		sprite = &pet_feed_very_bad_sprite;
	}
	else if(level > 5 || level > (food_active_count+1))
	{
		sprite = &pet_feed_good_sprite;
	}
	else if(level < food_active_count)
	{
		sprite = &pet_feed_bad_sprite;
	}
	CAT_draw_sprite(sprite, 0, x, y);
}

void render_arrangement()
{
	CAT_frameberry(0x53ab);

	CAT_draw_sprite(&feed_upper_tray_sprite, 0, menu_rect.min.x, menu_rect.min.y);
	CAT_draw_sprite(&feed_lower_tray_sprite, 0, table_rect.min.x, table_rect.min.y);

	if(show_gizmos)
	{
		int menu_x = menu_rect.min.x;
		int menu_y = menu_rect.min.y;
		int menu_w = menu_rect.max.x - menu_x;
		int menu_h = menu_rect.max.y - menu_y;
		CAT_strokeberry(menu_x, menu_y, menu_w, menu_h, CAT_RED);
		int table_x = table_rect.min.x;
		int table_y = table_rect.min.y;
		int table_w = table_rect.max.x - table_x;
		int table_h = table_rect.max.y - table_y;
		CAT_strokeberry(table_x, table_y, table_w, table_h, CAT_BLUE);
	}

	for(int i = food_idxs_l.length-1; i >= 0; i--)
	{
		CAT_item* food = food_get(i);
		CAT_rect rect = food_rects[i];
		int x = rect.min.x;
		int y = rect.min.y;
		int w = rect.max.x - x;
		int h = rect.max.y - y;
		CAT_push_draw_scale(3);
		CAT_push_draw_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_BOTTOM);
		CAT_draw_sprite(food->sprite, 0, x+w/2, y+h);
		
		if(show_gizmos)
		{
			if(food_active_count > 0)
				CAT_strokeberry(food_centroid.x-4, food_centroid.y-4, 8, 8, CAT_RED);

			if(food_active_mask[i])
			{
				CAT_RGB888 red = CAT_RGB24(255, 0, 0);
				CAT_RGB888 green = CAT_RGB24(0, 255, 0);
				uint16_t evenness_colour = CAT_RGB8882565(CAT_RGB888_lerp(red, green, evenness_score));
				CAT_lineberry(food_centers[i].x, food_centers[i].y, food_centroid.x, food_centroid.y, evenness_colour);
				int j = food_neighbour_map[i];
				if(food_active_mask[j])
					CAT_lineberry(food_centers[i].x, food_centers[i].y, food_centers[j].x, food_centers[j].y, evenness_colour);
				
				CAT_circberry(x+w/2, y+h/2, w/2, food_collision_mask[i] ? CAT_RED : CAT_WHITE);
			}
		}
		if(show_debug_text)
		{
			if(food_active_mask[i])
				render_text(x, y-14, CAT_WHITE, 1, "%d: %0.2f", i, food_angles[i]);
		}
	}

	CAT_push_draw_flags(CAT_DRAW_FLAG_BOTTOM | CAT_DRAW_FLAG_CENTER_X);
	if(show_feedback)
		CAT_push_draw_colour(RGB8882565(64, 64, 64));
	CAT_draw_sprite(&pet_feed_back_sprite, -1, 120, 320);
	if(show_feedback)
		render_feedback();

	if(show_debug_text)
	{
		CAT_gui_printf(CAT_WHITE, "group diversity: %0.2f", group_score);
		CAT_gui_printf(CAT_WHITE, "role propriety: %0.2f", role_score);
		CAT_gui_printf(CAT_WHITE, "ichiju sansai: %0.2f", ichisan_score);
		CAT_gui_printf(CAT_WHITE, "spacing: %0.2f", spacing_score);
		CAT_gui_printf(CAT_WHITE, "evenness: %0.2f", evenness_score);
		CAT_gui_printf(CAT_WHITE, "aggregate: %0.2f", aggregate_score);
		CAT_gui_printf(CAT_WHITE, "level: %d", level);
	}
}

void render_summary()
{
	CAT_frameberry(0xef39);

	const char* title;
	int stamp_idx;

	switch (summary_page)
	{
		case PERFORMANCE:
			title = "Performance";
			stamp_idx = level;
		break;
		case VARIETY:
			title = "Variety";
			stamp_idx = round(group_score * 5);
		break;
		case PROPRIETY:
			title = "Propriety";
			stamp_idx = round(role_score * 5);
		break;
		case LAYOUT:
			title = "Layout";
			stamp_idx = round((spacing_score + evenness_score) * 0.5f * 5);
		break;
		default:
			return;
		break;
	}

	int title_len = strlen(title);
	int title_x = (CAT_LCD_SCREEN_W - 1 - title_len * 16) / 2;
	render_text(title_x, 12, CAT_BLACK, 2, title);
	CAT_push_draw_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y);
	CAT_push_draw_colour(RGB8882565(128, 128, 128));
	CAT_draw_sprite(&feed_stamp_frame_sprite, 0, 120, 160);
	CAT_push_draw_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y);
	CAT_push_draw_colour(grade_colours[stamp_idx]);
	CAT_draw_sprite(&feed_grade_stamps_sprite, stamp_idx, 120+stamp_jitters[summary_page].x, 160+stamp_jitters[summary_page].y);

	CAT_push_draw_colour(RGB8882565(64, 64, 64));
	CAT_draw_sprite(&ui_left_arrow_sprite, -1, 8, 12);
	CAT_push_draw_colour(RGB8882565(64, 64, 64));
	CAT_draw_sprite(&ui_right_arrow_sprite, -1, 240-13-8, 12);
}

void CAT_MS_feed(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			CAT_set_render_callback(CAT_render_feed);

			CAT_item_list_init(&food_pool);
			CAT_item_list_filter(&bag, &food_pool, food_filter);

			CAT_int_list idx_pool_l;
			CAT_ilist(&idx_pool_l, idx_pool, CAT_ITEM_LIST_MAX_LENGTH);
			for(int i = 0; i < food_pool.length; i++)
				CAT_ilist_push(&idx_pool_l, i);
			CAT_ilist_shuffle(&idx_pool_l);
			
			CAT_ilist(&food_idxs_l, food_idxs, 5);
			for(int i = 0; i < min(idx_pool_l.length, 5); i++)
				food_spawn(idx_pool[i]);

			mode = ARRANGE;
			group_score = 0;
			role_score = 0;
			ichisan_score = 0;
			spacing_score = 0;
			evenness_score = 0;
			aggregate_score = 0;
			level = 0;
			commit = false;

			scroll_offset = 0;
			last_selected = -1;
			scrolling = false;
			CAT_timer_reinit(&inspect_timer_id, 1.0f);

			CAT_timer_reinit(&feedback_timer_id, 1.5f);
		break;
		case CAT_MACHINE_SIGNAL_TICK:
			switch(mode)
			{
				case SELECT:
				{
					if(CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START) || CAT_input_pressed(CAT_BUTTON_SELECT))
					{
						mode = ARRANGE;
						break;
					}

					select_grid_io();
					break;
				}
				case INSPECT:
				{
					if(CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
					{
						mode = SELECT;
						break;
					}
					else if(CAT_input_pressed(CAT_BUTTON_SELECT))
					{
						mode = ARRANGE;
						break;
					}

					if(CAT_input_pressed(CAT_BUTTON_RIGHT))
						inspect_idx += 1;
					if(CAT_input_pressed(CAT_BUTTON_LEFT))
						inspect_idx -= 1;
					inspect_idx = (inspect_idx + food_pool.length) % food_pool.length;
					break;
				}
				case ARRANGE:
				{		
					if(CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
						CAT_machine_back();

					if(CAT_input_pressed(CAT_BUTTON_SELECT))
					{
						mode = SELECT;
						break;
					}

					if(CAT_gui_popup_is_open())
						break;
					if(CAT_input_pressed(CAT_BUTTON_A))
						CAT_gui_open_popup("Submit this meal?\nFood items on table\nwill be consumed!\n", &commit);
					if(commit)
					{
						commit = false;
						bool empty = true;
						for(int i = 0; i < food_idxs_l.length; i++)
						{
							if(food_active_mask[i])
							{
								CAT_item_list_remove(&bag, food_pool.item_ids[food_idxs[i]], 1);
								empty = false;
							}
						}
						if(!empty)
							pet.vigour += level;

						mode = SUMMARY;
						summary_page = PERFORMANCE;
						for(int i = 0; i < SUMMARY_PAGE_MAX; i++)
						{
							stamp_jitters[i] = (CAT_vec2){CAT_rand_int(-5, 5), CAT_rand_int(-5, 5)};
						}
					}

					if(CAT_input_pressed(CAT_BUTTON_RIGHT))
						show_debug_text = !show_debug_text;
					if(CAT_input_pressed(CAT_BUTTON_LEFT))
						show_gizmos = !show_gizmos;

					if(!CAT_input_touching())
					{
						if(CAT_input_touch_up())
						{
							food_refresh();
							map_neighbours();
							score_refresh();
						}
						touched = -1;
					}
					else if(CAT_input_touch_down())
					{
						for(int i = 0; i < food_idxs_l.length; i++)
						{
							int x = food_rects[i].min.x;
							int y = food_rects[i].min.y;
							int w = food_rects[i].max.x - x;
							int h = food_rects[i].max.y - y;
							if(CAT_input_touch_rect(x, y, w, h))
							{
								food_swap(i, 0);
								dx = x - input.touch.x;
								dy = y - input.touch.y;	
								touched = 0;
								break;
							}
						}

						if(CAT_input_touch_rect(85, 240, 64, 64))
						{
							show_feedback = true;
							CAT_timer_reset(feedback_timer_id);
						}
					}
					else
					{
						if(touched != -1)
						{
							int w = food_rects[touched].max.x - food_rects[touched].min.x;
							int h = food_rects[touched].max.y - food_rects[touched].min.y;
							food_rects[touched].min.x = input.touch.x + dx;
							food_rects[touched].min.y = input.touch.y + dy;
							food_rects[touched].max.x = food_rects[touched].min.x + w;
							food_rects[touched].max.y = food_rects[touched].min.y + h;
						}
					}

					if(show_feedback && CAT_timer_tick(feedback_timer_id))
						show_feedback = false;
					break;
				}
				case SUMMARY:
				{
					if
					(
						CAT_input_pressed(CAT_BUTTON_A) ||
						CAT_input_pressed(CAT_BUTTON_B) ||
						CAT_input_pressed(CAT_BUTTON_START) ||
						CAT_input_pressed(CAT_BUTTON_SELECT)
					)
					{
						CAT_machine_back();
					}

					if(CAT_input_pressed(CAT_BUTTON_RIGHT))
						summary_page += 1;
					if(CAT_input_pressed(CAT_BUTTON_LEFT))
						summary_page -= 1;
					summary_page = (summary_page + SUMMARY_PAGE_MAX) % SUMMARY_PAGE_MAX;
					break;
				}
			}			
		break;
		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}

void CAT_render_feed()
{
	switch(mode)
	{
		case SELECT:
		{
			render_select_grid();
			break;
		}
		case INSPECT:
		{
			render_inspector();
			break;
		}
		case ARRANGE:
		{
			render_arrangement();
			break;
		}
		case SUMMARY:
		{
			render_summary();
			break;
		}
	}
}