#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
	const char* string;
	uint64_t tags;
} CAT_notice;

void CAT_set_notice_mask(uint64_t mask);
void CAT_post_notice();
void CAT_cancel_notice();
bool CAT_is_notice_posted();