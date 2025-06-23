#include "cat_crisis.h"

#include "cat_core.h"
#include "cat_aqi.h"
#include "cat_render.h"
#include "cat_input.h"
#include "cat_gui.h"

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
static float crisis_timer = 0;
static CAT_AQ_crisis_response_type crisis_response_type;
static bool crisis_notice = false;
static uint64_t crisis_timestamp;

void CAT_AQ_start_crisis(CAT_AQ_crisis_type type, CAT_AQ_crisis_severity severity)
{
	crisis_type = type;
	crisis_severity = severity;
	crisis_timer = 0;
	crisis_response_type = CAT_AQ_CRISIS_RESPONSE_TYPE_NONE;
	crisis_notice = true;
	crisis_timestamp = CAT_get_rtc_now();
}

void CAT_AQ_stop_crisis(CAT_AQ_crisis_response_type response_type)
{
	crisis_response_type = response_type;
}

bool CAT_AQ_is_crisis_ongoing()
{
	return
	crisis_type != CAT_AQ_CRISIS_TYPE_NONE &&
	crisis_response_type == CAT_AQ_CRISIS_RESPONSE_TYPE_NONE;
}

void CAT_AQ_crisis_tick()
{
	if(crisis_type != CAT_AQ_CRISIS_TYPE_NONE && CAT_AQ_is_crisis_ongoing())
	{
		crisis_timer += CAT_get_delta_time_s();
		if(CAT_AQ_poll_crisis_severity(crisis_type) == CAT_AQ_CRISIS_SEVERITY_NONE)
			CAT_AQ_stop_crisis(CAT_AQ_CRISIS_RESPONSE_TYPE_AUTOMATIC);
	}
}

int CAT_AQ_get_crisis_primetime()
{
	return crisis_primetimes[crisis_severity];
}

int CAT_AQ_get_crisis_uptime()
{
	return crisis_timer;
}

int CAT_AQ_get_crisis_overtime()
{
	return crisis_timer - CAT_AQ_get_crisis_primetime();
}

CAT_AQ_crisis_response_grade CAT_AQ_grade_crisis_response()
{
	int primetime = CAT_AQ_get_crisis_primetime();
	int overtime = crisis_timer - primetime;
	
	if(overtime <= 0)
	{
		if(crisis_timer <= CAT_MINUTE_SECONDS*5)
			return CAT_AQ_CRISIS_RESPONSE_GRADE_EXCELLENT;
		return CAT_AQ_CRISIS_RESPONSE_GRADE_ADEQUATE;
	}
	else if(overtime <= primetime * 2)
		return CAT_AQ_CRISIS_RESPONSE_GRADE_INADEQUATE;
	else
		return CAT_AQ_CRISIS_RESPONSE_GRADE_DISASTROUS;
}

bool CAT_AQ_is_crisis_notice_posted()
{
	return crisis_notice;
}

void CAT_AQ_dismiss_crisis_notice()
{
	crisis_notice = false;
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

const char* CAT_AQ_get_crisis_title()
{
	return crisis_titles[crisis_type];
}

const char* CAT_AQ_get_crisis_severity_string()
{
	return crisis_severity_strings[crisis_severity];
}

void CAT_AQ_poll_crisis()
{
	if(!CAT_AQ_is_crisis_ongoing())
	{
		CAT_AQ_crisis_type type = CAT_AQ_poll_crisis_type();
		if(type != CAT_AQ_CRISIS_TYPE_NONE)
		{
			CAT_AQ_crisis_severity severity = CAT_AQ_poll_crisis_severity(type);
			if(severity != CAT_AQ_CRISIS_SEVERITY_NONE)
				CAT_AQ_start_crisis(type, severity);
		}
	}
	else
	{
		CAT_AQ_crisis_tick();	
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTICE

void CAT_MS_crisis_notice(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			CAT_set_render_callback(CAT_render_crisis_notice);
		break;

		case CAT_MACHINE_SIGNAL_TICK:
			if(CAT_input_dismissal())
			{
				CAT_AQ_dismiss_crisis_notice();
				CAT_machine_back();
			}
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}

void CAT_render_crisis_notice()
{
	CAT_gui_panel((CAT_ivec2){0, 0}, (CAT_ivec2){15, 20});
	CAT_gui_textf(CAT_AQ_get_crisis_title());
	CAT_gui_line_break();
	CAT_gui_textf(CAT_AQ_get_crisis_severity_string());
	CAT_gui_line_break();
	
	CAT_datetime datetime;
	CAT_make_datetime(crisis_timestamp, &datetime);
	CAT_gui_textf("%.2d/%.2d/%.4d %.2d:%.2d:%.2d", datetime.month, datetime.day, datetime.year, datetime.hour, datetime.minute, datetime.second);
	CAT_gui_line_break();
	CAT_gui_textf("%d/%d", CAT_AQ_get_crisis_uptime(), CAT_AQ_get_crisis_primetime());
}