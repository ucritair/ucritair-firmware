#include "cat_crisis.h"

#include "cat_core.h"
#include "cat_aqi.h"
#include "cat_render.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// CRISIS

static const float crisis_severity_cutoffs[] =
{
	// NONE, MILD, MODERATE, SEVERE
	1.0f, 0.5f, 0.35f, 0.25f,
};

static const int crisis_primetimes[] =
{
	0, // NONE
	CAT_MINUTE_SECONDS * 30, // MILD
	CAT_MINUTE_SECONDS * 20, // MODERATE
	CAT_MINUTE_SECONDS * 10 // SEVERE
};

CAT_AQ_crisis_type CAT_AQ_poll_crisis_type()
{
	if(!CAT_is_AQ_initialized())
		return CAT_AQ_CRISIS_TYPE_NONE;

	int worst_aqm = -1;
	float worst_score = __FLT_MAX__;
	for(int i = 0; i < CAT_AQM_AGGREGATE; i++)
	{
		float score = CAT_AQ_get_normalized_score(i);
		if(score < 0.5f && score < worst_score)
		{
			worst_aqm = i;
			worst_score = score;
		}
	}
		
	switch(worst_aqm)
	{
		case CAT_AQM_CO2: return CAT_AQ_CRISIS_TYPE_CO2;
		case CAT_AQM_PM2_5: return CAT_AQ_CRISIS_TYPE_PM2_5;
		case CAT_AQM_NOX:
		case CAT_AQM_VOC: return CAT_AQ_CRISIS_TYPE_NOX_VOC;
		case CAT_AQM_TEMP:
		case CAT_AQM_RH: return CAT_AQ_CRISIS_TYPE_TEMP_RH;
		default: return CAT_AQ_CRISIS_TYPE_NONE;
	};
}

float get_crisis_score(CAT_AQ_crisis_type type)
{
	switch(type)
	{
		case CAT_AQ_CRISIS_TYPE_CO2: return CAT_AQ_get_normalized_score(CAT_AQM_CO2);
		case CAT_AQ_CRISIS_TYPE_PM2_5: return CAT_AQ_get_normalized_score(CAT_AQM_PM2_5);
		case CAT_AQ_CRISIS_TYPE_NOX_VOC: return min(CAT_AQ_get_normalized_score(CAT_AQM_NOX), CAT_AQ_get_normalized_score(CAT_AQM_VOC));
		case CAT_AQ_CRISIS_TYPE_TEMP_RH: return min(CAT_AQ_get_normalized_score(CAT_AQM_TEMP), CAT_AQ_get_normalized_score(CAT_AQM_RH));
		default: return 1.0f;
	}
};

CAT_AQ_crisis_severity CAT_AQ_poll_crisis_severity(CAT_AQ_crisis_type type)
{	
	float score = get_crisis_score(type);

	for(int s = CAT_AQ_CRISIS_SEVERITY_EXTREME; s >= CAT_AQ_CRISIS_SEVERITY_NONE; s--)
	{
		float cutoff = crisis_severity_cutoffs[s];
		if(score <= cutoff)
			return s;
	}
	
	return CAT_AQ_CRISIS_SEVERITY_NONE;
}

static CAT_AQ_crisis_type crisis_type = CAT_AQ_CRISIS_TYPE_NONE;
static CAT_AQ_crisis_severity crisis_severity = CAT_AQ_CRISIS_SEVERITY_NONE;
static bool crisis_ongoing = false;
static bool crisis_notice = false;

static uint64_t crisis_start_timestamp = 0;
static uint64_t crisis_end_timestamp = 0;
static uint64_t crisis_notice_dismiss_timestamp = 0;
static float crisis_timer = 0;

static CAT_AQ_crisis_response_type crisis_response_type = CAT_AQ_CRISIS_RESPONSE_TYPE_NONE;
static CAT_AQ_crisis_response_grade crisis_response_grade = CAT_AQ_CRISIS_RESPONSE_GRADE_NONE;
static uint8_t lifetime_damage = 0;

void CAT_AQ_start_crisis(CAT_AQ_crisis_type type, CAT_AQ_crisis_severity severity)
{
	crisis_type = type;
	crisis_severity = severity;
	crisis_ongoing = true;
	crisis_notice = true;

	crisis_start_timestamp = CAT_get_rtc_now();
	crisis_end_timestamp = 0;
	crisis_notice_dismiss_timestamp = 0;
	crisis_timer = 0;

	crisis_response_type = CAT_AQ_CRISIS_RESPONSE_TYPE_NONE;
	crisis_response_grade = CAT_AQ_CRISIS_RESPONSE_GRADE_NONE;
	lifetime_damage = 0;
}

bool CAT_AQ_is_crisis_ongoing()
{
	return crisis_ongoing;
}

int CAT_AQ_get_crisis_primetime()
{
	return 10;//crisis_primetimes[crisis_severity];
}

int CAT_AQ_get_crisis_uptime()
{
	return crisis_timer;
}

int CAT_AQ_get_crisis_overtime()
{
	return crisis_timer - CAT_AQ_get_crisis_primetime();
}

uint64_t CAT_AQ_get_crisis_start()
{
	return crisis_start_timestamp;
}

uint64_t CAT_AQ_get_crisis_end()
{
	return crisis_end_timestamp;
}

void CAT_AQ_stop_crisis(CAT_AQ_crisis_response_type response_type)
{
	crisis_ongoing = false;
	crisis_end_timestamp = CAT_get_rtc_now();
	crisis_response_type = response_type;
	
	int primetime = CAT_AQ_get_crisis_primetime();
	int overtime = crisis_timer - primetime;
	crisis_response_grade =
	overtime <= (-primetime / 2) ? CAT_AQ_CRISIS_RESPONSE_GRADE_EXCELLENT :
	overtime <= 0 ? CAT_AQ_CRISIS_RESPONSE_GRADE_ADEQUATE :
	overtime <= primetime ? CAT_AQ_CRISIS_RESPONSE_GRADE_INADEQUATE :
	CAT_AQ_CRISIS_RESPONSE_GRADE_DISASTROUS;
}

CAT_AQ_crisis_response_grade CAT_AQ_grade_crisis_response()
{
	return crisis_response_grade;
}

int CAT_AQ_get_crisis_lifetime_damage()
{
	return lifetime_damage;
}

bool CAT_AQ_is_crisis_waiting()
{
	return
	!crisis_ongoing &&
	crisis_notice;
}

void CAT_AQ_crisis_tick()
{
	if(!CAT_AQ_is_crisis_ongoing())
	{
		if(!CAT_AQ_is_crisis_waiting())
		{
			uint64_t time_since_last_crisis = CAT_get_rtc_now() - crisis_notice_dismiss_timestamp;
			if(time_since_last_crisis >= 5)
			{
				CAT_AQ_crisis_type type = CAT_AQ_poll_crisis_type();
				if(type != CAT_AQ_CRISIS_TYPE_NONE)
				{
					CAT_AQ_crisis_severity severity = CAT_AQ_poll_crisis_severity(type);
					if(severity != CAT_AQ_CRISIS_SEVERITY_NONE)
						CAT_AQ_start_crisis(type, severity);
				}
			}
		}
	}
	else
	{
		crisis_timer += CAT_get_delta_time_s();
		if(CAT_AQ_poll_crisis_severity(crisis_type) == CAT_AQ_CRISIS_SEVERITY_NONE || crisis_timer >= CAT_AQ_get_crisis_primetime())
		{
			CAT_AQ_stop_crisis(CAT_AQ_CRISIS_RESPONSE_TYPE_AUTOMATIC);
		}
	}
}

bool CAT_AQ_is_crisis_notice_posted()
{
	return crisis_notice;
}

void CAT_AQ_dismiss_crisis_notice()
{
	crisis_notice = false;
	crisis_notice_dismiss_timestamp = CAT_get_rtc_now();
}

static const char* crisis_titles[] =
{
	"N/A",
	"MIASMA",
	"PARTICULATE STORM",
	"REAGENT STORM",
	"THERMICS"
};

static const char* crisis_severity_strings[] =
{
	"N/A",
	"MILD",
	"MODERATE",
	"SEVERE",
};

static const char* crisis_response_type_strings[] =
{
	"N/A",
	"MANUAL",
	"NATURAL",
	"EQUIPMENT",
};

static const char* crisis_response_grade_strings[] =
{
	"N/A",
	"DISASTROUS",
	"INADEQUATE",
	"ADEQUATE",
	"EXCELLENT",
};

const char* CAT_AQ_get_crisis_title()
{
	return crisis_titles[crisis_type];
}

const char* CAT_AQ_get_crisis_severity_string()
{
	return crisis_severity_strings[crisis_severity];
}

const char* CAT_AQ_get_crisis_response_type_string()
{
	return crisis_response_type_strings[crisis_response_type];
}

const char* CAT_AQ_get_crisis_response_grade_string()
{
	return crisis_response_grade_strings[crisis_response_grade];
}
