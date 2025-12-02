#include <stdio.h>
#include <stdint.h>
#include <time.h>

uint64_t CAT_get_RTC_offset()
{
	return 0;
}

uint64_t CAT_get_RTC_now()
{
	time_t t = time(NULL);
	struct tm tm;
	localtime_r(&t, &tm);
	tm.tm_year += 1900;
	return timegm(&tm);
}

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

void print(CAT_datetime* t)
{
	printf("%.2d/%.2d/%.4d %.2d:%.2d:%.2d\n", t->month, t->day, t->year, t->hour, t->minute, t->second);
}

int main()
{
	uint64_t t = CAT_get_RTC_now();
	CAT_datetime now;
	CAT_make_datetime(t, &now);
	now.day -= 24;
	now = CAT_normalize_date(now);
	print(&now);
	return 0;
}
