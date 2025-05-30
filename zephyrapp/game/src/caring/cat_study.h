#pragma once

#include "cat_machine.h"
#include <stdint.h>

typedef struct
{
	const char* name;
	const char* proverb;
	int grade_constraint;

	float min_length;
	float max_length;
	float min_lustre;
	float max_lustre;
	float min_wisdom;
	float max_wisdom;

	uint8_t* vertices;
	int vertex_count;
} CAT_fish;

void CAT_MS_study(CAT_machine_signal signal);