#include "cat_time.h"

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

void CAT_get_earliest_date(CAT_datetime* datetime)
{
	CAT_log_cell first;
	CAT_read_first_calendar_cell(&first);
	CAT_make_datetime(first.timestamp, datetime);
}

int CAT_get_min_year()
{
	CAT_datetime earliest;
	CAT_get_earliest_date(&earliest);
	return earliest.year;
}
int CAT_get_max_year()
{
	CAT_datetime latest;
	CAT_get_datetime(&latest);
	return latest.year;
}

int CAT_get_min_month(int year)
{
	CAT_datetime earliest;
	CAT_get_earliest_date(&earliest);
	return year == earliest.year ? earliest.month : 1;
}
int CAT_get_max_month(int year)
{
	CAT_datetime latest;
	CAT_get_datetime(&latest);
	return year == latest.year ? latest.month : 12;
}

int CAT_get_min_day(int year, int month)
{
	CAT_datetime earliest;
	CAT_get_earliest_date(&earliest);
	return (year == earliest.year && month == earliest.month) ? earliest.day : 1;
}
int CAT_get_max_day(int year, int month)
{
	CAT_datetime latest;
	CAT_get_datetime(&latest);
	return (year == latest.year && month == latest.month) ? latest.day : CAT_days_in_month(year, month);
}

int CAT_clamp_date_part(int part, int year, int month, int day)
{
	switch(part)
	{
		case CAT_DATE_PART_YEAR: return clamp(year, CAT_get_min_year(), CAT_get_max_year());
		case CAT_DATE_PART_MONTH: return clamp(month, CAT_get_min_month(year), CAT_get_max_month(year));
		case CAT_DATE_PART_DAY: return clamp(day, CAT_get_min_day(year, month), CAT_get_max_day(year, month));
	}
	return -1;
}