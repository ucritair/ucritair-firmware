#pragma once

#include "cat_core.h"

#include "cat_time.h"

bool CAT_is_leap_year(int year);
int CAT_days_in_month(int year, int month);
int CAT_sakamoto_weekday(int year, int month, int day);

void CAT_get_earliest_date(CAT_datetime* datetime);

int CAT_get_min_year();
int CAT_get_max_year();
int CAT_get_min_month(int year);
int CAT_get_max_month(int year);
int CAT_get_min_day(int year, int month);
int CAT_get_max_day(int year, int month);

typedef enum
{
	CAT_DATE_PART_YEAR,
	CAT_DATE_PART_MONTH,
	CAT_DATE_PART_DAY
} CAT_date_part;

int CAT_clamp_date_part(int part, int year, int month, int day);