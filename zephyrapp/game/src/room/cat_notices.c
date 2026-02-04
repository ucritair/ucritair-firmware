#include "cat_notices.h"

#include "stdint.h"
#include "cat_core.h"
#include "cat_gui.h"
#include <stdio.h>
#include "notice_assets.h"

static uint64_t mask = 0;

void CAT_set_notice_mask(uint64_t _mask)
{
	mask = _mask;
}

static const CAT_notice* select_notice()
{
	int suitable = 0;
	if(mask == CAT_CONTENT_TAG_NONE)
		suitable = CAT_NOTICE_COUNT;
	else
	{
		for(int i = 0; i < CAT_NOTICE_COUNT; i++)
		{
			if((CAT_notice_list[i].tags & mask) == 0)
				suitable += 1;
		}
	}

	int suitable_count = 0;
	int suitable_idx = CAT_rand_int(0, suitable-1);
	for(int i = 0; i < CAT_NOTICE_COUNT; i++)
	{
		if((CAT_notice_list[i].tags & mask) == 0)
		{
			if(suitable_count == suitable_idx)
				return &CAT_notice_list[i];
			suitable_count += 1;
		}
	}
	return NULL;
}

static CAT_notice* notice = NULL;

void CAT_post_notice()
{
	notice = select_notice();
	if(notice == NULL)
		return;
}

void CAT_cancel_notice()
{
	notice = NULL;
}

bool CAT_is_notice_posted()
{
	return notice != NULL;
}