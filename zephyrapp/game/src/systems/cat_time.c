#include "cat_time.h"

#include <time.h>
#include <cat_core.h>
#include "cat_math.h"

void CAT_get_datetime(CAT_datetime* datetime)
{
	time_t now = CAT_get_RTC_now();
	struct tm local;
	gmtime_r(&now, &local);
	
	datetime->year = local.tm_year;
	datetime->month = local.tm_mon+1;
	datetime->day = local.tm_mday;
	datetime->hour = local.tm_hour;
	datetime->minute = local.tm_min;
	datetime->second = local.tm_sec;	
}

int CAT_datecmp(CAT_datetime* a, CAT_datetime* b)
{
	if(a->year > b->year)
		return 1;
	if(a->year < b->year)
		return -1;
	if(a->month > b->month)
		return 1;
	if(a->month < b->month)
		return -1;
	if(a->day > b->day)
		return 1;
	if(a->day < b->day)
		return -1;
	return 0;
}

int CAT_timecmp(CAT_datetime* a, CAT_datetime* b)
{
	if(a->year > b->year)
		return 1;
	if(a->year < b->year)
		return -1;
	if(a->month > b->month)
		return 1;
	if(a->month < b->month)
		return -1;
	if(a->day > b->day)
		return 1;
	if(a->day < b->day)
		return -1;
	if(a->hour > b->hour)
		return 1;
	if(a->hour < b->hour)
		return -1;
	if(a->minute > b->minute)
		return 1;
	if(a->minute < b->minute)
		return -1;
	if(a->second > b->second)
		return 1;
	if(a->second < b->second)
		return -1;
	return 0;
}

void CAT_make_datetime(uint64_t timestamp, CAT_datetime* datetime)
{
	struct tm t = {0};
	gmtime_r(&timestamp, &t);
	datetime->year = t.tm_year;
	datetime->month = t.tm_mon+1;
	datetime->day = t.tm_mday;
	datetime->hour = t.tm_hour;
	datetime->minute = t.tm_min;
	datetime->second = t.tm_sec;
}

uint64_t CAT_make_timestamp(CAT_datetime* datetime)
{
	struct tm t = {0};
	t.tm_year = datetime->year;
	t.tm_mon = datetime->month-1;
	t.tm_mday = datetime->day;
	t.tm_hour = datetime->hour;
	t.tm_min = datetime->minute;
	t.tm_sec = datetime->second;
	return timegm(&t) + CAT_get_RTC_offset();
}

CAT_datetime CAT_normalize_date(CAT_datetime date)
{
	uint64_t t = CAT_make_timestamp(&date);
	CAT_make_datetime(t, &date);
	return date;
}

bool CAT_is_leap_year(int year)
{
	return (year % 4) || ((year % 100 == 0) && (year % 400)) ? 0 : 1;
}

int CAT_days_in_month(int year, int month)
{
	return month == 2 ? (28 + CAT_is_leap_year(year)) : 31 - (month-1) % 7 % 2;
}

int CAT_sakamoto_weekday(int year, int month, int day)
{
    // Sakamoto's algorithm (Gregorian)
    const int t[] = {0,3,2,5,0,3,5,1,4,6,2,4};
    year -= month < 3; // make Jan/Feb part of previous year for leap-day handling
    int w = (year + year/4 - year/100 + year/400 + t[month-1] + day) % 7;
    if (w < 0) w += 7;
    return w;
}

static CAT_datetime clip_min;
static CAT_datetime clip_max;

void CAT_set_date_clip_min(CAT_datetime min)
{
	clip_min = min;
}

void CAT_set_date_clip_max(CAT_datetime max)
{
	clip_max = max;
}

static bool date_eq_up_to(CAT_datetime a, CAT_datetime b, int part)
{
	for(int i = 0; i <= part; i++)
	{
		if(a.data[i] != b.data[i])
			return false;
	}
	return true;
}

static int general_min(CAT_datetime date, int part)
{
	switch (part)
	{
		case CAT_DATE_PART_YEAR: return 0;
		case CAT_DATE_PART_MONTH: return 1;
		case CAT_DATE_PART_DAY: return 1;
		case CAT_DATE_PART_HOUR:
		case CAT_DATE_PART_MINUTE:
		case CAT_DATE_PART_SECOND: return 0;
	}
}

static int general_max(CAT_datetime date, int part)
{
	switch (part)
	{
		case CAT_DATE_PART_YEAR: return INT32_MAX;
		case CAT_DATE_PART_MONTH: return 12;
		case CAT_DATE_PART_DAY: return CAT_days_in_month(date.year, date.month);
		case CAT_DATE_PART_HOUR: return 23;
		case CAT_DATE_PART_MINUTE:
		case CAT_DATE_PART_SECOND: return 59;
	}
}

int CAT_clip_date_part(CAT_datetime date, int part)
{
	if(part == CAT_DATE_PART_YEAR)
		return CAT_clamp(date.year, clip_min.year, clip_max.year);
	int min = date_eq_up_to(date, clip_min, part-1) ? clip_min.data[part] : general_min(date, part);
	int max = date_eq_up_to(date, clip_max, part-1) ? clip_max.data[part] : general_max(date, part);
	return CAT_clamp(date.data[part], min, max);
}

CAT_datetime CAT_clip_date(CAT_datetime date)
{
	CAT_normalize_date(date);
	for(int i = 0; i < CAT_DATE_PART_COUNT; i++)
		date.data[i] = CAT_clip_date_part(date, i);
	return date;
}

void CAT_get_first_date(CAT_datetime* datetime)
{
	CAT_log_cell first;
	CAT_read_first_calendar_cell(&first);
	CAT_make_datetime(first.timestamp, datetime);
}