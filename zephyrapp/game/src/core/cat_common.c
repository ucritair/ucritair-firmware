#include "cat_core.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// SCREEN MANAGEMENT

static CAT_screen_orientation screen_orientation = CAT_SCREEN_ORIENTATION_UP;

void CAT_set_screen_orientation(CAT_screen_orientation orientation)
{
	screen_orientation = orientation;
}

CAT_screen_orientation CAT_get_screen_orientation()
{
	return screen_orientation;
}
