#pragma once

#include <stdbool.h>

typedef enum
{
	CAT_NOTICE_TYPE_AQ_GOOD,
	CAT_NOTICE_TYPE_AQ_BAD,
	CAT_NOTICE_TYPE_STATS_GOOD,
	CAT_NOTICE_TYPE_STATS_BAD,
	CAT_NOTICE_TYPE_SPRING,
	CAT_NOTICE_TYPE_SUMMER,
	CAT_NOTICE_TYPE_AUTUMN,
	CAT_NOTICE_TYPE_WINTER, 
	CAT_NOTICE_TYPE_MORNING,
	CAT_NOTICE_TYPE_DAY,
	CAT_NOTICE_TYPE_NIGHT,
	CAT_NOTICE_TYPE_DEAD,
	CAT_NOTICE_TYPE_MISCELLANY,
	CAT_NOTICE_TYPE_COUNT,
} CAT_notice_type;

void CAT_clear_notice_types();
void CAT_enable_notice_type(int type);
int CAT_pick_notice_type();

bool CAT_should_post_notice();
const char* CAT_pick_notice(int type);
void CAT_post_notice(const char* notice);