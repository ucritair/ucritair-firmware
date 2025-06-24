#pragma once

#include "stdbool.h"
#include "cat_machine.h"
#include "stdint.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// CRISIS

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
	CAT_AQ_CRISIS_RESPONSE_GRADE_NONE,
	CAT_AQ_CRISIS_RESPONSE_GRADE_DISASTROUS,
	CAT_AQ_CRISIS_RESPONSE_GRADE_INADEQUATE,
	CAT_AQ_CRISIS_RESPONSE_GRADE_ADEQUATE,
	CAT_AQ_CRISIS_RESPONSE_GRADE_EXCELLENT
} CAT_AQ_crisis_response_grade;

CAT_AQ_crisis_type CAT_AQ_poll_crisis_type();
CAT_AQ_crisis_severity CAT_AQ_poll_crisis_severity(CAT_AQ_crisis_type type);

void CAT_AQ_start_crisis(CAT_AQ_crisis_type type, CAT_AQ_crisis_severity severity);

bool CAT_AQ_is_crisis_ongoing();
int CAT_AQ_get_crisis_primetime();
int CAT_AQ_get_crisis_uptime();
int CAT_AQ_get_crisis_overtime();

uint64_t CAT_AQ_get_crisis_start();
uint64_t CAT_AQ_get_crisis_end();

void CAT_AQ_stop_crisis(CAT_AQ_crisis_response_type response_type);

CAT_AQ_crisis_response_grade CAT_AQ_grade_crisis_response();
int CAT_AQ_get_crisis_lifespan_damage();
bool CAT_AQ_is_crisis_waiting();

void CAT_AQ_crisis_tick();

bool CAT_AQ_is_crisis_notice_posted();
void CAT_AQ_dismiss_crisis_notice();

const char* CAT_AQ_get_crisis_title();
const char* CAT_AQ_get_crisis_severity_string();
const char* CAT_AQ_get_crisis_response_type_string();
const char* CAT_AQ_get_crisis_response_grade_string();


////////////////////////////////////////////////////////////////////////////////////////////////////
// REPORT

#define CRISIS_RED 0xea01
#define CRISIS_YELLOW 0xfd45
#define CRISIS_GREEN 0x5d6d

void CAT_MS_crisis_report(CAT_machine_signal signal);
void CAT_render_crisis_report();


