#pragma once

#include <stdint.h>

// PPM CO2 mapped from f32 to 8 bits of u32
uint32_t CAT_ZK_CO2();
// UG/M3 PM 2.5 mapped from f32 to 8 bits of u32
uint32_t CAT_ZK_PM2_5();
// Degrees celsius mapped from f32 to 8 bits of u32
uint32_t CAT_ZK_temp();
// % Correct mapped from f32 to 8 bits of u32
uint32_t CAT_ZK_stroop();
// Survey information as 8 bits of u32
uint32_t CAT_ZK_survey();