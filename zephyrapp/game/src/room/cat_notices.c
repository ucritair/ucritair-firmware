#include "cat_notices.h"

#include "stdint.h"
#include "cat_core.h"
#include "cat_gui.h"
#include <stdio.h>

#define NOTICE_COOLDOWN (CAT_MINUTE_SECONDS)
#define NOTICE_DURATION 2

static const char** notice_pool[CAT_NOTICE_TYPE_COUNT] =
{
	[CAT_NOTICE_TYPE_AQ_GOOD] = (const char*[])
	{
		"A fresh cool breeze blows",
		"The critter breathes gently",
		"There is a faint scent of flowers on the air",
		NULL,
	},
	[CAT_NOTICE_TYPE_AQ_BAD] = (const char*[])
	{
		"An acrid wind blows",
		"The critter gasps for air",
		"A chemical smell permeates the air",
		NULL
	},
	[CAT_NOTICE_TYPE_STATS_GOOD] = (const char*[])
	{
		"The critter flexes, energetic and ready",
		"The critter is thinking about something intently",
		"The critter dances playfully",
		NULL,
	},
	[CAT_NOTICE_TYPE_STATS_BAD] = (const char*[])
	{
		"The critter's shoulders slump with exhaustion",
		"The critter is distractedly fidgeting",
		"The critter sports a sour look",
		NULL,
	},
	[CAT_NOTICE_TYPE_SPRING] = (const char*[])
	{
		"Crickets sing a droning song",
		"A chickadee's call tumbles over itself",
		"A bluejay's cry pierces the cool air",
		NULL,
	},
	[CAT_NOTICE_TYPE_SUMMER] = (const char*[])
	{
		"The calling of cicadas penetrates the stones",
		"A warm wind cushions you",
		"Somewhere, waves crash on sand",
		NULL,
	},
	[CAT_NOTICE_TYPE_AUTUMN] = (const char*[])
	{
		"Leaves rustle somewhere nearby",
		"A cool and dry wind blows a twig over stones",
		"An owl looses a mournful call",
		NULL,
	},
	[CAT_NOTICE_TYPE_WINTER] = (const char*[])
	{
		"Snow falls, blanketing the world",
		"What light remains glitters in the ice",
		"The smell of spices warms you",
		NULL,
	},
	[CAT_NOTICE_TYPE_MORNING] = (const char*[])
	{
		"Sunlight beams in from the world",
		"The waking city calls out to you",
		"A train speeds its charges to work",
		"A rooster crows proudly",
		NULL,
	},
	[CAT_NOTICE_TYPE_DAY] = (const char*[])
	{
		"The critter yawns prematurely",
		"A shop door jingles as it opens",
		"A freight train's great body slithers past",
		"A man calls out for a taxi",
		NULL,
	},
	[CAT_NOTICE_TYPE_NIGHT] = (const char*[])
	{
		"Jazz wafts in from a dimly lit bar",
		"The sound of laughter and clinking glass is heard",
		"A cat meows just once at the moon",
		"The last train speeds home",
		NULL,
	},
	[CAT_NOTICE_TYPE_DEAD] = (const char*[])
	{
		"A grave egg feels no pain",
		NULL,
	},
	[CAT_NOTICE_TYPE_MISCELLANY] = (const char*[])
	{
		"A distant bell sounds",
		"An evil star is shining",
		"A salamander scurries into flame to be destroyed",
		"The dead dream that they live",
		"The vending machine hums softly",
		"The arcade lets loose a single beep",
		"A smaller critter scurries across the floor",
		NULL,
	},
};

static uint64_t notice_timestamp;

static int notice_type_list_backing[CAT_NOTICE_TYPE_COUNT];
static CAT_int_list notice_type_list;

void CAT_clear_notice_types()
{
	CAT_ilist(&notice_type_list, notice_type_list_backing, CAT_NOTICE_TYPE_COUNT);
}

void CAT_enable_notice_type(int type)
{
	CAT_ilist_push(&notice_type_list, type);
}

int CAT_pick_notice_type()
{
	if(notice_type_list.length == 0)
		return CAT_rand_int(0, CAT_NOTICE_TYPE_COUNT-1);
	int idx = CAT_rand_int(0, notice_type_list.length-1);
	return notice_type_list.data[idx];
}

bool CAT_should_post_notice()
{
	int time_since = CAT_get_RTC_now() - notice_timestamp;
	return time_since >= NOTICE_COOLDOWN;
}

const char* CAT_pick_notice(int type)
{
	if(type == -1)
		type = CAT_rand_int(CAT_NOTICE_TYPE_AQ_GOOD, CAT_NOTICE_TYPE_COUNT-1);
	const char** sector = notice_pool[type];
	int count = 0;
	while(sector[count++] != NULL);
	count -= 1;
	int idx = CAT_rand_int(0, count-1);
	if(idx < 0)
		return NULL;
	return sector[idx];
}

static char notice_buf[128];

void CAT_post_notice(const char* notice)
{
	if(notice == NULL)
		return;
	CAT_gui_dismiss_dialogue();
	snprintf(notice_buf, 128, "%s...", notice);
	CAT_gui_open_dialogue(notice_buf, NOTICE_DURATION);
	notice_timestamp = CAT_get_RTC_now();
}