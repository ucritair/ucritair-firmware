#pragma once

#include "cat_core.h"

void imu_init();
void imu_update();

extern bool imu_ok;
extern bool imu_posted;

extern CAT_IMU_values imu_raw;
extern float imu_magnitude;
extern CAT_IMU_values imu_normalized;