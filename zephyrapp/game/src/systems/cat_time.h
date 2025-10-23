#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef union
{
	struct
	{
		int
		year, // [0, INF)
		month, // [1, 12]
		day, // [1, 31]
		hour, // [0, 24)
		minute, // [0, 60)
		second; // [0, 60)
	};
	
   	int data[6];
} CAT_datetime;

typedef enum
{
	CAT_DATE_PART_YEAR,
	CAT_DATE_PART_MONTH,
	CAT_DATE_PART_DAY,
	CAT_DATE_PART_HOUR,
	CAT_DATE_PART_MINUTE,
	CAT_DATE_PART_SECOND,
	CAT_DATE_PART_COUNT
} CAT_date_part;

#define NULL_DATE (CAT_datetime) {0, 0, 0, 0, 0, 0};

void CAT_get_datetime(CAT_datetime* datetime);
int CAT_datecmp(CAT_datetime* a, CAT_datetime* b);
int CAT_timecmp(CAT_datetime* a, CAT_datetime* b);
void CAT_make_datetime(uint64_t timestamp, CAT_datetime* datetime);
uint64_t CAT_make_timestamp(CAT_datetime* datetime);

CAT_datetime CAT_normalize_date(CAT_datetime date);

bool CAT_is_leap_year(int year);
int CAT_days_in_month(int year, int month);
int CAT_sakamoto_weekday(int year, int month, int day);

void CAT_set_date_clip_min(CAT_datetime min);
void CAT_set_date_clip_max(CAT_datetime max);
int CAT_clip_date_part(CAT_datetime date, int part);
CAT_datetime CAT_clip_date(CAT_datetime in);

void CAT_get_first_date(CAT_datetime* date);