#include "cat_effects.h"

#include "cat_core.h"
#include "cat_render.h"

static float duration = 0;
static float direction = 0;
static float timer = 0;

void CAT_effect_start_aperture_blackout(float _duration, float _direction)
{
	duration = _duration;
	direction = _direction;
	timer = 0;
}

void CAT_effect_cancel_aperture_blackout()
{
	duration = 0;
	direction = 0;
	timer = 0;
}

bool CAT_effect_poll_aperture_blackout()
{
	if(duration == 0 || direction == 0)
		return false;
	return timer < duration;
}

void tick_aperture_blackout()
{
	float dt = CAT_min(CAT_get_delta_time_s(), 0.07);
	timer += dt;
	timer = CAT_min(timer, duration);
}

void render_aperture_blackout()
{
	if(!CAT_effect_poll_aperture_blackout())
		return;

	float t = timer/duration;
	t = direction > 0 ? t : 1.0f - t;
	int r = 200 * t;
	int r2 = r * r;

	for(int y = 0; y < CAT_LCD_SCREEN_H; y++)
	{
		int dy = y - CAT_LCD_SCREEN_H/2;
		for(int x = 0; x < CAT_LCD_SCREEN_W; x++)
		{
			int dx = x - CAT_LCD_SCREEN_W/2;
			int d2 = dy*dy + dx*dx;
			if(d2 > r2)
				CAT_pixberry(x, y, CAT_BLACK);
		}
	}
}

void CAT_effects_tick()
{
	tick_aperture_blackout();
}

void CAT_effects_render()
{
	render_aperture_blackout();
}