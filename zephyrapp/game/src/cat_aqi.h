#pragma once

#include "cat_core.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// SCORING

float CAT_CO2_score(float co2);
float CAT_VOC_score(float voc);
float CAT_NOX_score(float nox);
float CAT_PM2_5_score(float pm25);
float CAT_canonical_temp();
float CAT_temp_score(float temp);
float CAT_rh_score(float rh);
float CAT_IAQ_score(float co2, float voc, float nox, float pm25, float temp, float rh);
float CAT_AQ_aggregate_score();

typedef enum
{
	CAT_AQM_CO2,
	CAT_AQM_PM2_5,
	CAT_AQM_NOX,
	CAT_AQM_VOC,
	CAT_AQM_TEMP,
	CAT_AQM_RH,
	CAT_AQM_AGGREGATE,
	CAT_AQM_COUNT,
} CAT_AQM;

static const char* CAT_AQM_titles[] =
{
	[CAT_AQM_CO2] = "CO2",
	[CAT_AQM_PM2_5] = "PM2.5",
	[CAT_AQM_NOX] = "NOX",
	[CAT_AQM_VOC] = "VOC",
	[CAT_AQM_TEMP] = "TEMP",
	[CAT_AQM_RH] = "RH",
	[CAT_AQM_AGGREGATE] = "\4CritAQ Score"
};

const char* CAT_get_AQM_unit_string(int aqm);

float CAT_AQ_get_raw_score(int aqm);
void CAT_AQ_store_raw_scores();

float CAT_AQ_get_normalized_score(int aqm);
void CAT_AQ_store_normalized_scores();


////////////////////////////////////////////////////////////////////////////////////////////////////
// CRISES

typedef enum
{
	CAT_AQ_CRISIS_TYPE_NONE,
	CAT_AQ_CRISIS_TYPE_CO2,
	CAT_AQ_CRISIS_TYPE_PM2_5,
	CAT_AQ_CRISIS_TYPE_NOX_VOC,
	CAT_AQ_CRISIS_TYPE_TEMP_RH
} CAT_AQ_crisis_type;

typedef enum
{
	CAT_AQ_CRISIS_SEVERITY_NONE,
	CAT_AQ_CRISIS_SEVERITY_MILD,
	CAT_AQ_CRISIS_SEVERITY_MODERATE,
	CAT_AQ_CRISIS_SEVERITY_EXTREME
} CAT_AQ_crisis_severity;

typedef enum
{
	CAT_AQ_CRISIS_RESPONSE_TYPE_NONE,
	CAT_AQ_CRISIS_RESPONSE_TYPE_MANUAL,
	CAT_AQ_CRISIS_RESPONSE_TYPE_AUTOMATIC,
	CAT_AQ_CRISIS_RESPONSE_TYPE_ASSISTED
} CAT_AQ_crisis_response_type;

typedef enum
{
	CAT_AQ_CRISIS_RESPONSE_GRADE_DISASTROUS,
	CAT_AQ_CRISIS_RESPONSE_GRADE_INADEQUATE,
	CAT_AQ_CRISIS_RESPONSE_GRADE_ADEQUATE,
	CAT_AQ_CRISIS_RESPONSE_GRADE_EXCELLENT
} CAT_AQ_crisis_response_grade;

CAT_AQ_crisis_type CAT_AQ_poll_crisis();
CAT_AQ_crisis_severity CAT_AQ_poll_crisis_severity(CAT_AQ_crisis_type type);

void CAT_AQ_start_crisis(CAT_AQ_crisis_type type, CAT_AQ_crisis_severity severity);
void CAT_AQ_stop_crisis(CAT_AQ_crisis_response_type response_type);
bool CAT_AQ_is_crisis_ongoing();
void CAT_AQ_crisis_tick();

int CAT_AQ_get_crisis_primetime();
int CAT_AQ_get_crisis_duration();
int CAT_AQ_get_crisis_overtime();
CAT_AQ_crisis_response_grade CAT_AQ_grade_crisis_response();

bool CAT_AQ_is_crisis_notice_posted();
void CAT_AQ_dismiss_crisis_notice();

const char* CAT_AQ_get_crisis_title();
const char* CAT_AQ_get_crisis_severity_string();

void CAT_AQ_tick();

