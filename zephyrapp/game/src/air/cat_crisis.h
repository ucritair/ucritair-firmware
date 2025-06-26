#pragma once

#include "stdbool.h"
#include "cat_machine.h"
#include "stdint.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// CRISIS

typedef enum
{
	CAT_AQ_CRISIS_TYPE_NONE = 0,
	CAT_AQ_CRISIS_TYPE_CO2 = 1,
	CAT_AQ_CRISIS_TYPE_PM2_5 = 2,
	CAT_AQ_CRISIS_TYPE_NOX_VOC = 3,
	CAT_AQ_CRISIS_TYPE_TEMP_RH = 4
} CAT_AQ_crisis_type;

typedef enum
{
	CAT_AQ_CRISIS_SEVERITY_NONE = 0,
	CAT_AQ_CRISIS_SEVERITY_MILD = 1,
	CAT_AQ_CRISIS_SEVERITY_MODERATE = 2,
	CAT_AQ_CRISIS_SEVERITY_EXTREME = 3
} CAT_AQ_crisis_severity;

typedef enum
{
	CAT_AQ_CRISIS_RESPONSE_TYPE_NONE = 0,
	CAT_AQ_CRISIS_RESPONSE_TYPE_MANUAL = 1,
	CAT_AQ_CRISIS_RESPONSE_TYPE_AUTOMATIC = 2,
	CAT_AQ_CRISIS_RESPONSE_TYPE_ASSISTED = 3
} CAT_AQ_crisis_response_type;

typedef enum
{
	CAT_AQ_CRISIS_RESPONSE_GRADE_NONE = 0,
	CAT_AQ_CRISIS_RESPONSE_GRADE_DISASTROUS = 1,
	CAT_AQ_CRISIS_RESPONSE_GRADE_INADEQUATE = 2,
	CAT_AQ_CRISIS_RESPONSE_GRADE_ADEQUATE = 3,
	CAT_AQ_CRISIS_RESPONSE_GRADE_EXCELLENT = 4
} CAT_AQ_crisis_response_grade;

typedef struct __attribute__((__packed__))
{
	CAT_AQ_crisis_type type;
	CAT_AQ_crisis_severity severity;
	CAT_AQ_crisis_severity peak_severity;
	bool ongoing;
	bool report;

	uint64_t start_timestamp;
	uint64_t peak_timestamp;
	uint64_t end_timestamp;

	CAT_AQ_crisis_response_type response_type;
	CAT_AQ_crisis_response_grade response_grade;
	uint8_t lifespan_damage;
} CAT_AQ_crisis_state;

void CAT_AQ_export_crisis_state(CAT_AQ_crisis_state* out);
void CAT_AQ_import_crisis_state(CAT_AQ_crisis_state* in);

CAT_AQ_crisis_type CAT_AQ_poll_crisis_type();
CAT_AQ_crisis_severity CAT_AQ_poll_crisis_severity(CAT_AQ_crisis_type type);

void CAT_AQ_start_crisis(CAT_AQ_crisis_type type, CAT_AQ_crisis_severity severity);

bool CAT_AQ_is_crisis_ongoing();
int CAT_AQ_get_crisis_total_uptime();
int CAT_AQ_get_crisis_peak_uptime();
int CAT_AQ_get_crisis_avoidance_window();
int CAT_AQ_get_crisis_disaster_uptime();

uint64_t CAT_AQ_get_crisis_start();
uint64_t CAT_AQ_get_crisis_end();

void CAT_AQ_stop_crisis(CAT_AQ_crisis_response_type response_type);

CAT_AQ_crisis_type CAT_AQ_get_crisis_type();
CAT_AQ_crisis_response_type CAT_AQ_get_crisis_response_type();
CAT_AQ_crisis_response_grade CAT_AQ_get_crisis_response_grade();
int CAT_AQ_get_crisis_lifespan_damage();
CAT_AQ_crisis_severity CAT_AQ_get_crisis_peak_severity();
bool CAT_AQ_is_crisis_waiting();

void CAT_AQ_crisis_tick();

void CAT_AQ_post_crisis_report();
bool CAT_AQ_is_crisis_report_posted();
void CAT_AQ_dismiss_crisis_report();

const char* CAT_AQ_crisis_type_string(int type);
const char* CAT_AQ_crisis_severity_string(int severity);
const char* CAT_AQ_crisis_response_type_string(int type);
const char* CAT_AQ_crisis_response_grade_string(int grade);


////////////////////////////////////////////////////////////////////////////////////////////////////
// REPORT

void CAT_MS_crisis_report(CAT_machine_signal signal);
void CAT_render_crisis_report();


