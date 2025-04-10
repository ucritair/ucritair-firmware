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


////////////////////////////////////////////////////////////////////////////////////////////////////
// SAVE

static uint16_t session_flags = 0;

uint16_t CAT_get_save_flags()
{
	return session_flags;
}

void CAT_set_save_flags(uint16_t flags)
{
	session_flags = flags;
}

void CAT_enable_save_flag(CAT_save_flag flag)
{
	session_flags |= (1 << flag);
}

void CAT_disable_save_flag(CAT_save_flag flag)
{
	session_flags &= ~(1 << flag);
}

bool CAT_is_save_flag_enabled(CAT_save_flag flag)
{
	return (session_flags & (1 << flag)) > 0;
}