#include "cat_crisis.h"

#include "cat_core.h"
#include "cat_air.h"
#include "cat_render.h"
#include "cat_item.h"
#include "item_assets.h"
#include "cat_room.h"
#include "cat_pet.h"
#include <math.h>
#include "cat_persist.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// CRISIS

static const float crisis_severity_cutoffs[] =
{
	// NONE, MILD, MODERATE, SEVERE
	1.0f, 0.5f, 0.35f, 0.25f,
};

static const int crisis_avoidance_windows[] =
{
	0, // NONE
	CAT_MINUTE_SECONDS * 30, // MILD
	CAT_MINUTE_SECONDS * 10, // MODERATE
	CAT_MINUTE_SECONDS * 5 // SEVERE
};

CAT_AQ_crisis_type CAT_AQ_poll_crisis_type()
{
	if(!CAT_AQ_sensors_initialized())
		return CAT_AQ_CRISIS_TYPE_NONE;

	int worst_aqm = -1;
	float worst_score = __FLT_MAX__;
	for(int i = 0; i < CAT_AQM_AGGREGATE; i++)
	{
		float score = CAT_AQ_live_score_normalized(i);
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
		case CAT_AQ_CRISIS_TYPE_CO2: return CAT_AQ_live_score_normalized(CAT_AQM_CO2);
		case CAT_AQ_CRISIS_TYPE_PM2_5: return CAT_AQ_live_score_normalized(CAT_AQM_PM2_5);
		case CAT_AQ_CRISIS_TYPE_NOX_VOC: return CAT_min(CAT_AQ_live_score_normalized(CAT_AQM_NOX), CAT_AQ_live_score_normalized(CAT_AQM_VOC));
		case CAT_AQ_CRISIS_TYPE_TEMP_RH: return CAT_min(CAT_AQ_live_score_normalized(CAT_AQM_TEMP), CAT_AQ_live_score_normalized(CAT_AQM_TEMP));
		default: return 1.0f;
	}
};

CAT_AQ_crisis_severity CAT_AQ_poll_crisis_severity(CAT_AQ_crisis_type type)
{	
	switch(type)
	{
		case CAT_AQ_CRISIS_TYPE_TEMP_RH:
		{
			float T_d = CAT_AQ_live_score_raw(CAT_AQM_TEMP);
			float rh = CAT_AQ_live_score_raw(CAT_AQM_RH);
			float wb = CAT_wet_bulb_temp(T_d);

			// Dangerous wet-bulb temps are roughly 32C and over
			if(wb >= 32)
				return CAT_AQ_CRISIS_SEVERITY_MODERATE;
			return CAT_AQ_CRISIS_SEVERITY_NONE;
		}
		break;

		case CAT_AQ_CRISIS_TYPE_NOX_VOC:
		{
			return CAT_AQ_CRISIS_SEVERITY_NONE;
		}
		break;

		default:
		{
			float score = get_crisis_score(type);
			for(int s = CAT_AQ_CRISIS_SEVERITY_EXTREME; s >= CAT_AQ_CRISIS_SEVERITY_NONE; s--)
			{
				float cutoff = crisis_severity_cutoffs[s];
				if(score <= cutoff)
					return s;
			}
		}
		break;
	}
	
	return CAT_AQ_CRISIS_SEVERITY_NONE;
}

static CAT_AQ_crisis_state crisis = {0};
void CAT_AQ_export_crisis_state(volatile CAT_AQ_crisis_state* out)
{
	memcpy(out, &crisis, sizeof(CAT_AQ_crisis_state));
}
void CAT_AQ_import_crisis_state(volatile CAT_AQ_crisis_state* in)
{
	memcpy(&crisis, in, sizeof(CAT_AQ_crisis_state));
}

void CAT_AQ_start_crisis(CAT_AQ_crisis_type type, CAT_AQ_crisis_severity severity)
{
	crisis.type = type;
	crisis.severity = severity;
	crisis.peak_severity = severity;
	crisis.ongoing = true;
	crisis.report = true;

	crisis.start_timestamp = CAT_get_RTC_now();
	crisis.peak_timestamp = CAT_get_RTC_now();
	crisis.end_timestamp = 0;

	crisis.response_type = CAT_AQ_CRISIS_RESPONSE_TYPE_NONE;
	crisis.response_grade = CAT_AQ_CRISIS_RESPONSE_GRADE_NONE;
	crisis.lifespan_damage = 0;
}

bool CAT_AQ_is_crisis_ongoing()
{
	return crisis.ongoing;
}

int CAT_AQ_get_crisis_avoidance_window()
{
	switch (crisis.type)
	{
		case CAT_AQ_CRISIS_TYPE_TEMP_RH:
			return 2 * CAT_HOUR_SECONDS;
		break;

		default:
			crisis_avoidance_windows[crisis.severity];
		break;
	}
}

int CAT_AQ_get_crisis_total_uptime()
{
	if(CAT_AQ_is_crisis_ongoing())
		return CAT_get_RTC_now() - crisis.start_timestamp;
	return crisis.end_timestamp - crisis.start_timestamp;
}

int CAT_AQ_get_crisis_peak_uptime()
{
	if(CAT_AQ_is_crisis_ongoing())
		return CAT_get_RTC_now() - crisis.peak_timestamp;
	return crisis.end_timestamp - crisis.peak_timestamp;
}

int CAT_AQ_get_crisis_disaster_uptime()
{
	return CAT_AQ_get_crisis_peak_uptime() - crisis_avoidance_windows[crisis.peak_severity];
}

uint64_t CAT_AQ_get_crisis_start()
{
	return crisis.start_timestamp;
}

uint64_t CAT_AQ_get_crisis_end()
{
	return crisis.end_timestamp;
}

void CAT_AQ_stop_crisis(CAT_AQ_crisis_response_type response_type)
{
	if(!crisis.ongoing)
		return;
		
	crisis.ongoing = false;
	crisis.report = true;
	crisis.end_timestamp = CAT_get_RTC_now();
	crisis.response_type = response_type;
	
	int window = crisis_avoidance_windows[crisis.peak_severity];
	int overtime = CAT_AQ_get_crisis_disaster_uptime();
	float response_ratio = (float) overtime / (float) window;
	crisis.response_grade =
	response_ratio <= 0.5f ? CAT_AQ_CRISIS_RESPONSE_GRADE_EXCELLENT :
	response_ratio <= 1.0f ? CAT_AQ_CRISIS_RESPONSE_GRADE_ADEQUATE :
	response_ratio < 2.0f ? CAT_AQ_CRISIS_RESPONSE_GRADE_INADEQUATE :
	CAT_AQ_CRISIS_RESPONSE_GRADE_DISASTROUS;

	if(!(persist_flags & CAT_PERSIST_CONFIG_FLAG_PAUSE_CARE))
	{
		if
		(
			crisis.peak_severity >= CAT_AQ_CRISIS_SEVERITY_EXTREME &&
			crisis.response_grade < CAT_AQ_CRISIS_RESPONSE_GRADE_EXCELLENT
		)
		{
			crisis.lifespan_damage += 1;
		}

		crisis.lifespan_damage +=
		crisis.response_grade <= CAT_AQ_CRISIS_RESPONSE_GRADE_DISASTROUS ? 2 :
		crisis.response_grade <= CAT_AQ_CRISIS_RESPONSE_GRADE_INADEQUATE ? 1 :
		0;

		pet.lifespan -= crisis.lifespan_damage;
	}
}

CAT_AQ_crisis_type CAT_AQ_get_crisis_type()
{
	return crisis.type;
}

CAT_AQ_crisis_response_type CAT_AQ_get_crisis_response_type()
{
	return crisis.response_type;
}

CAT_AQ_crisis_response_grade CAT_AQ_get_crisis_response_grade()
{
	return crisis.response_grade;
}

int CAT_AQ_get_crisis_lifespan_damage()
{
	return crisis.lifespan_damage;
}

CAT_AQ_crisis_severity CAT_AQ_get_crisis_peak_severity()
{
	return crisis.peak_severity;
}

bool CAT_AQ_is_crisis_waiting()
{
	return
	!crisis.ongoing &&
	crisis.report;
}

void CAT_AQ_crisis_tick()
{
	if(!CAT_AQ_sensors_initialized())
		return;

	if(!CAT_AQ_is_crisis_ongoing())
	{
		if(!CAT_AQ_is_crisis_waiting())
		{
			CAT_AQ_crisis_type type = CAT_AQ_poll_crisis_type();
			CAT_AQ_crisis_severity severity = CAT_AQ_poll_crisis_severity(type);

			if(type != CAT_AQ_CRISIS_TYPE_NONE && severity != CAT_AQ_CRISIS_SEVERITY_NONE)
			{
				uint64_t time_since_last_crisis = CAT_get_RTC_now() - crisis.end_timestamp;
				uint16_t threshold = CAT_HOUR_SECONDS;

				if(type == crisis.type)
				{
					if(type == CAT_AQ_CRISIS_TYPE_TEMP_RH)
						threshold = CAT_HOUR_SECONDS * 4;
					else
						threshold = CAT_HOUR_SECONDS * 2;
				}

				if(time_since_last_crisis >= threshold)
					CAT_AQ_start_crisis(type, severity);
			}
		}
	}
	else
	{
		CAT_AQ_crisis_severity current_severity = CAT_AQ_poll_crisis_severity(crisis.type);

		// AUTOMATIC RESPONSE
		if(current_severity == CAT_AQ_CRISIS_SEVERITY_NONE)
		{
			CAT_AQ_stop_crisis(CAT_AQ_CRISIS_RESPONSE_TYPE_AUTOMATIC);
			return;
		}

		if(current_severity > crisis.peak_severity)
		{
			crisis.peak_severity = current_severity;
			crisis.peak_timestamp = CAT_get_RTC_now();
		}
		crisis.severity = current_severity;

		// ITEM ASSISTED RESPONSES
		if(crisis.type == CAT_AQ_CRISIS_TYPE_CO2)
		{
			int uv_idx = CAT_room_prop_lookup(prop_uv_lamp_item);
			if(CAT_pushdown_peek() == &CAT_MS_room && uv_idx != -1)
			{
				CAT_room_remove_prop(uv_idx);
				CAT_AQ_stop_crisis(CAT_AQ_CRISIS_RESPONSE_TYPE_ASSISTED);
			}
		}
		else if(crisis.type == CAT_AQ_CRISIS_TYPE_PM2_5 || crisis.type == CAT_AQ_CRISIS_TYPE_NOX_VOC)
		{
			if(crisis.severity <= CAT_AQ_CRISIS_SEVERITY_MILD)
			{
				if(CAT_inventory_count(mask_item))
				{
					CAT_inventory_remove(mask_item, 1);
					CAT_AQ_stop_crisis(CAT_AQ_CRISIS_RESPONSE_TYPE_ASSISTED);
				}
			}
			else
			{
				int pure_idx = CAT_room_prop_lookup(prop_purifier_item);
				if(CAT_pushdown_peek() == &CAT_MS_room && pure_idx != -1)
				{
					CAT_room_remove_prop(pure_idx);
					CAT_AQ_stop_crisis(CAT_AQ_CRISIS_RESPONSE_TYPE_ASSISTED);
				}
			}
		}
	}
}

void CAT_AQ_post_crisis_report()
{
	if(CAT_AQ_is_crisis_ongoing() || CAT_AQ_is_crisis_waiting())
		crisis.report = true;
}

bool CAT_AQ_is_crisis_report_posted()
{
	return crisis.report;
}

void CAT_AQ_dismiss_crisis_report()
{
	crisis.report = false;
}

static const char* crisis_type_strings[] =
{
	"N/A",
	"CARBON DIOXIDE",
	"PARTICULATE MATTER",
	"VOLATILE COMPOUNDS",
	"WET BULB"
};

static const char* crisis_severity_strings[] =
{
	"N/A",
	"MILD",
	"MODERATE",
	"EXTREME",
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

const char* CAT_AQ_crisis_type_string(int type)
{
	if(type == -1)
		type = crisis.type;
	return crisis_type_strings[type];
}

const char* CAT_AQ_crisis_severity_string(int severity)
{
	if(severity == -1)
		severity = crisis.severity;
	return crisis_severity_strings[severity];
}

const char* CAT_AQ_crisis_response_type_string(int type)
{
	if(type == -1)
		type = crisis.response_type;
	return crisis_response_type_strings[type];
}

const char* CAT_AQ_crisis_response_grade_string(int grade)
{
	if(grade == -1)
		grade = crisis.response_grade;
	return crisis_response_grade_strings[grade];
}
