/*
 * THIS FILE IS AUTOMATICALLY GENERATED
 *
 * I2C-Generator: 0.3.0
 * Yaml Version: 2.1.3
 * Template Version: 0.7.0-109-gb259776
 */
/*
 * Copyright (c) 2021, Sensirion AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Sensirion AG nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SEN5X_I2C_H
#define SEN5X_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sensirion_config.h"

/**
 * sen5x_start_measurement() - Starts a continuous measurement.
 *
 * After starting the measurement, it takes some time (~1s) until the first
 * measurement results are available. You could poll with the command
 * 0x0202 \"Read Data Ready\" to check when the results are ready to read.
 *
 * If the device is in measure mode without particulate matter (low-power)
 * and the firmware version is at least 2.0, this command enables PM
 * measurement without affecting the already running RH/T/VOC/NOx measurements
 * (except that the \"data ready\"-flag will be cleared). In previous firmware
 * versions, this command is supported only in idle mode.
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sen5x_start_measurement(void);

/**
 * sen5x_start_measurement_without_pm() - Starts a continuous measurement
 * without PM. Only humidity, temperature, VOC and NOx are available in this
 * mode. Laser and fan are switched off to keep power consumption low.
 *
 * After starting the measurement, it takes some time (~1s) until the first
 * measurement results are available. You could poll with the command
 * 0x0202 \"Read Data Ready\" to check when the results are ready to read.
 *
 * If the device is in measure mode with particulate matter (normal measure
 * mode) and the firmware version is at least 2.0, this command disables PM
 * measurement without affecting the already running RH/T/VOC/NOx measurements
 * (except that the \"data ready\"-flag will be cleared). In previous firmware
 * versions, this command is supported only in idle mode.
 *
 * Supported sensors: SEN54, SEN55
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sen5x_start_measurement_without_pm(void);

/**
 * sen5x_stop_measurement() - Stops the measurement and returns to idle mode.
 *
 * If the device is already in idle mode, this command has no effect.
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sen5x_stop_measurement(void);

/**
 * sen5x_read_data_ready() - This command can be used to check if new
 * measurement results are ready to read. The data ready flag is automatically
 * reset after reading the measurement values with the 0x03.. \"Read Measured
 * Values\" commands.
 *
 * @note During fan (auto-)cleaning, no measurement data is available for
 * several seconds and thus this flag will not be set until cleaning has
 * finished. So please expect gaps of several seconds at any time if fan
 * auto-cleaning is enabled.
 *
 * @param padding Padding byte, always 0x00.
 *
 * @param data_ready True (0x01) if data is ready, False (0x00) if not. When no
 * measurement is running, False will be returned.
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sen5x_read_data_ready(bool* data_ready);

/**
 * sen5x_read_measured_values() - Returns the measured values.
 *
 * The command 0x0202 \"Read Data Ready\" can be used to check if new
 * data is available since the last read operation. If no new data is
 * available, the previous values will be returned again. If no data is
 * available at all (e.g. measurement not running for at least one
 * second), all values will be at their upper limit (0xFFFF for `uint16`,
 * 0x7FFF for `int16`).
 *
 * @param mass_concentration_pm1p0 Value is scaled with factor 10:
 *                                 PM1.0 [µg/m³] = value / 10
 * @note If this value is unknown, 0xFFFF is returned.
 *
 * @param mass_concentration_pm2p5 Value is scaled with factor 10:
 *                                 PM2.5 [µg/m³] = value / 10
 * @note If this value is unknown, 0xFFFF is returned.
 *
 * @param mass_concentration_pm4p0 Value is scaled with factor 10:
 *                                 PM4.0 [µg/m³] = value / 10
 * @note If this value is unknown, 0xFFFF is returned.
 *
 * @param mass_concentration_pm10p0 Value is scaled with factor 10:
 *                                  PM10.0 [µg/m³] = value / 10
 * @note If this value is unknown, 0xFFFF is returned.
 *
 * @param ambient_humidity Value is scaled with factor 100: RH [%] = value / 100
 * @note If this value is unknown, 0x7FFF is returned.*
 *
 * @param ambient_temperature Value is scaled with factor 200:
 *                            T [°C] = value / 200
 * @note If this value is unknown, 0x7FFF is returned.*
 *
 * @param voc_index Value is scaled with factor 10: VOC Index = value / 10
 * @note If this value is unknown, 0x7FFF is returned.*
 *
 * @param nox_index Value is scaled with factor 10: NOx Index = value / 10
 * @note If this value is unknown, 0x7FFF is returned. During
 * the first 10..11 seconds after power-on or device reset, this
 * value will be 0x7FFF as well.*
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sen5x_read_measured_values(uint16_t* mass_concentration_pm1p0,
                                   uint16_t* mass_concentration_pm2p5,
                                   uint16_t* mass_concentration_pm4p0,
                                   uint16_t* mass_concentration_pm10p0,
                                   int16_t* ambient_humidity,
                                   int16_t* ambient_temperature,
                                   int16_t* voc_index, int16_t* nox_index);

/**
 * sen5x_read_measured_raw_values() - Returns the measured raw values.

The command 0x0202 \"Read Data Ready\" can be used to check if new
data is available since the last read operation. If no new data is
available, the previous values will be returned again. If no data
is available at all (e.g. measurement not running for at least one
second), all values will be at their upper limit (0xFFFF for `uint16`,
0x7FFF for `int16`).
 *
 * @param raw_humidity Value is scaled with factor 100: RH [%] = value / 100
 * @note If this value is unknown, 0x7FFF is returned.
 *
 * @param raw_temperature Value is scaled with factor 200: T [°C] = value / 200
 * @note If this value is unknown, 0x7FFF is returned.
 *
 * @param raw_voc Raw measured VOC ticks without scale factor.
 * @note If this value is unknown, 0xFFFF is returned.
 *
 * @param raw_nox Raw measured NOx ticks without scale factor.
 * @note If this value is unknown, 0xFFFF is returned. During
 * the first 10..11 seconds after power-on or device reset, this
 * value will be 0xFFFF as well.*
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sen5x_read_measured_raw_values(int16_t* raw_humidity,
                                       int16_t* raw_temperature,
                                       uint16_t* raw_voc, uint16_t* raw_nox);

/**
 * sen5x_read_measured_values_sen50() - Returns the measured values for SEN50.
 *
 * The command 0x0202 \"Read Data Ready\" can be used to check if new
 * data is available since the last read operation. If no new data is
 * available, the previous values will be returned again. If no data is
 * available at all (e.g. measurement not running for at least one
 * second), all values will be at their upper limit (0xFFFF).
 *
 * @param mass_concentration_pm1p0 Value is scaled with factor 10:
 *                                 PM1.0 [µg/m³] = value / 10
 * @note If this value is unknown, 0xFFFF is returned.
 *
 * @param mass_concentration_pm2p5 Value is scaled with factor 10:
 *                                 PM2.5 [µg/m³] = value / 10
 * @note If this value is unknown, 0xFFFF is returned.
 *
 * @param mass_concentration_pm4p0 Value is scaled with factor 10:
 *                                 PM4.0 [µg/m³] = value / 10
 * @note If this value is unknown, 0xFFFF is returned.
 *
 * @param mass_concentration_pm10p0 Value is scaled with factor 10:
 *                                  PM10.0 [µg/m³] = value / 10
 * @note If this value is unknown, 0xFFFF is returned.
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sen5x_read_measured_values_sen50(uint16_t* mass_concentration_pm1p0,
                                         uint16_t* mass_concentration_pm2p5,
                                         uint16_t* mass_concentration_pm4p0,
                                         uint16_t* mass_concentration_pm10p0);

/**
 * sen5x_read_measured_pm_values() - Returns the measured particulate matter
 * values.
 *
 * The command 0x0202 \"Read Data Ready\" can be used to check if new
 * data is available since the last read operation. If no new data is
 * available, the previous values will be returned again. If no data
 * is available at all (e.g. measurement not running for at least one
 * second), all values will be 0xFFFF.
 *
 * @param mass_concentration_pm1p0 Value is scaled with factor 10:
 *                                 PM1.0 [µg/m³] = value / 10
 * @note If this value is unknown, 0xFFFF is returned.
 *
 * @param mass_concentration_pm2p5 Value is scaled with factor 10:
 *                                 PM2.5 [µg/m³] = value / 10
 * @note If this value is unknown, 0xFFFF is returned.
 *
 * @param mass_concentration_pm4p0 Value is scaled with factor 10:
 *                                 PM4.0 [µg/m³] = value / 10
 * @note If this value is unknown, 0xFFFF is returned.
 *
 * @param mass_concentration_pm10p0 Value is scaled with factor 10:
 *                                  PM10.0 [µg/m³] = value / 10
 * @note If this value is unknown, 0xFFFF is returned.
 *
 * @param number_concentration_pm0p5 Value is scaled with factor 10:
 *                                   PM0.5 [#/cm³] = value / 10
 * @note If this value is unknown, 0xFFFF is returned.
 *
 * @param number_concentration_pm1p0 Value is scaled with factor 10:
 *                                   PM1.0 [#/cm³] = value / 10
 * @note If this value is unknown, 0xFFFF is returned.
 *
 * @param number_concentration_pm2p5 Value is scaled with factor 10:
 *                                   PM2.5 [#/cm³] = value / 10
 * @note If this value is unknown, 0xFFFF is returned.
 *
 * @param number_concentration_pm4p0 Value is scaled with factor 10:
 *                                   PM4.0 [#/cm³] = value / 10
 * @note If this value is unknown, 0xFFFF is returned.
 *
 * @param number_concentration_pm10p0 Value is scaled with factor 10:
 *                                    PM10.0 [#/cm³] = value / 10
 * @note If this value is unknown, 0xFFFF is returned.
 *
 * @param typical_particle_size Value is scaled with factor 1000:
 *                              Size [µm] = value / 1000
 * @note If this value is unknown, 0xFFFF is returned.
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sen5x_read_measured_pm_values(
    uint16_t* mass_concentration_pm1p0, uint16_t* mass_concentration_pm2p5,
    uint16_t* mass_concentration_pm4p0, uint16_t* mass_concentration_pm10p0,
    uint16_t* number_concentration_pm0p5, uint16_t* number_concentration_pm1p0,
    uint16_t* number_concentration_pm2p5, uint16_t* number_concentration_pm4p0,
    uint16_t* number_concentration_pm10p0, uint16_t* typical_particle_size);

/**
 * sen5x_start_fan_cleaning() - Starts the fan cleaning manually. The \"data
 * ready\"-flag will be cleared immediately and during the next few seconds, no
 * new measurement results will be available (old values will be returned). Once
 * the cleaning is finished, the \"data ready\"-flag will be set and new
 * measurement results will be available.
 *
 * When executing this command while cleaning is already active, the
 * command does nothing.
 *
 * If you stop the measurement while fan cleaning is active, the cleaning
 * will be aborted immediately.
 *
 * @note This command is only available in measure mode with PM measurement
 * enabled, i.e. only if the fan is already running. In any other state, this
 * command does nothing.
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sen5x_start_fan_cleaning(void);

/**
 * sen5x_set_temperature_offset_parameters() - Sets the temperature offset
 * parameters for the device.
 *
 * Supported sensors: SEN54, SEN55
 *
 * @param temp_offset Constant temperature offset scaled with factor 200 (T [°C]
 * = value / 200). The default value is 0.
 *
 * @param slope Normalized temperature offset slope scaled with factor 10000
 * (applied factor = value / 10000). The default value is 0.
 *
 * @param time_constant Time constant [s] how fast the new slope and offset will
 * be applied. After the specified value in seconds, 63% of the new slope and
 * offset are applied. A time constant of zero means the new values will be
 * applied immediately (within the next measure interval of 1 second).
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sen5x_set_temperature_offset_parameters(int16_t temp_offset,
                                                int16_t slope,
                                                uint16_t time_constant);

/**
 * sen5x_get_temperature_offset_parameters() - Gets the temperature offset
 * parameters from the device.
 *
 * Supported sensors: SEN54, SEN55
 *
 * @param temp_offset Constant temperature offset scaled with factor 200 (T [°C]
 * = value / 200).
 *
 * @param slope Normalized temperature offset slope scaled with factor 10000
 * (applied factor = value / 10000).
 *
 * @param time_constant Time constant [s] how fast the slope and offset are
 * applied. After the specified value in seconds, 63% of the new slope and
 * offset are applied.
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sen5x_get_temperature_offset_parameters(int16_t* temp_offset,
                                                int16_t* slope,
                                                uint16_t* time_constant);

/**
 * sen5x_set_warm_start_parameter() - Sets the warm start parameter for the
 * device.
 *
 * Supported sensors: SEN54, SEN55
 *
 * @note This parameter can be changed in any state of the device (and the
 * getter immediately returns the new value), but it is applied only the next
 * time starting a measurement, i.e. when sending a \"Start Measurement\"
 * command! So the parameter needs to be set *before* a warm-start measurement
 * is started.
 *
 * @param warm_start Warm start behavior as a value in the range from 0 (cold
 * start) to 65535 (warm start). The default value is 0.
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sen5x_set_warm_start_parameter(uint16_t warm_start);

/**
 * sen5x_get_warm_start_parameter() - Gets the warm start parameter from the
 * device.
 *
 * Supported sensors: SEN54, SEN55
 *
 * @param warm_start Warm start behavior as a value in the range from 0 (cold
 * start) to 65535 (warm start).
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sen5x_get_warm_start_parameter(uint16_t* warm_start);

/**
 * sen5x_set_voc_algorithm_tuning_parameters() - Sets the tuning parameters of
 * the VOC algorithm.
 *
 * Supported sensors: SEN54, SEN55
 *
 * @note This command is available only in idle mode. In measure mode, this
 * command has no effect. In addition, it has no effect if at least one
 * parameter is outside the specified range.
 *
 * @param index_offset VOC index representing typical (average) conditions.
 * Allowed values are in range 1..250. The default value is 100.
 *
 * @param learning_time_offset_hours Time constant to estimate the VOC algorithm
 * offset from the history in hours. Past events will be forgotten after about
 * twice the learning time. Allowed values are in range 1..1000. The default
 * value is 12 hours.
 *
 * @param learning_time_gain_hours Time constant to estimate the VOC algorithm
 * gain from the history in hours. Past events will be forgotten after about
 * twice the learning time. Allowed values are in range 1..1000. The default
 * value is 12 hours.
 *
 * @param gating_max_duration_minutes Maximum duration of gating in minutes
 * (freeze of estimator during high VOC index signal). Set to zero to disable
 * the gating. Allowed values are in range 0..3000. The default value is 180
 * minutes.
 *
 * @param std_initial Initial estimate for standard deviation. Lower value
 * boosts events during initial learning period, but may result in larger
 * device-to-device variations. Allowed values are in range 10..5000. The
 * default value is 50.
 *
 * @param gain_factor Gain factor to amplify or to attenuate the VOC index
 * output. Allowed values are in range 1..1000. The default value is 230.
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sen5x_set_voc_algorithm_tuning_parameters(
    int16_t index_offset, int16_t learning_time_offset_hours,
    int16_t learning_time_gain_hours, int16_t gating_max_duration_minutes,
    int16_t std_initial, int16_t gain_factor);

/**
 * sen5x_get_voc_algorithm_tuning_parameters() - Gets the currently set tuning
 * parameters of the VOC algorithm.
 *
 * Supported sensors: SEN54, SEN55
 *
 * @param index_offset VOC index representing typical (average) conditions.
 *
 * @param learning_time_offset_hours Time constant to estimate the VOC algorithm
 * offset from the history in hours. Past events will be forgotten after about
 * twice the learning time.
 *
 * @param learning_time_gain_hours Time constant to estimate the VOC algorithm
 * gain from the history in hours. Past events will be forgotten after about
 * twice the learning time.
 *
 * @param gating_max_duration_minutes Maximum duration of gating in minutes
 * (freeze of estimator during high VOC index signal). Zero disables the gating.
 *
 * @param std_initial Initial estimate for standard deviation. Lower value
 * boosts events during initial learning period, but may result in larger
 * device-to-device variations.
 *
 * @param gain_factor Gain factor to amplify or to attenuate the VOC index
 * output.
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sen5x_get_voc_algorithm_tuning_parameters(
    int16_t* index_offset, int16_t* learning_time_offset_hours,
    int16_t* learning_time_gain_hours, int16_t* gating_max_duration_minutes,
    int16_t* std_initial, int16_t* gain_factor);

/**
 * sen5x_set_nox_algorithm_tuning_parameters() - Sets the tuning parameters of
 * the NOx algorithm.
 *
 * Supported sensors: SEN55
 *
 * @note This command is available only in idle mode. In measure mode, this
 * command has no effect. In addition, it has no effect if at least one
 * parameter is outside the specified range.
 *
 * @param index_offset NOx index representing typical (average) conditions.
 * Allowed values are in range 1..250. The default value is 1.
 *
 * @param learning_time_offset_hours Time constant to estimate the NOx algorithm
 * offset from the history in hours. Past events will be forgotten after about
 * twice the learning time. Allowed values are in range 1..1000. The default
 * value is 12 hours.
 *
 * @param learning_time_gain_hours The time constant to estimate the NOx
 * algorithm gain from the history has no impact for NOx. This parameter is
 * still in place for consistency reasons with the VOC tuning parameters
 * command. This parameter must always be set to 12 hours.
 *
 * @param gating_max_duration_minutes Maximum duration of gating in minutes
 * (freeze of estimator during high NOx index signal). Set to zero to disable
 * the gating. Allowed values are in range 0..3000. The default value is 720
 * minutes.
 *
 * @param std_initial The initial estimate for standard deviation parameter has
 * no impact for NOx. This parameter is still in place for consistency reasons
 * with the VOC tuning parameters command. This parameter must always be set
 * to 50.
 *
 * @param gain_factor Gain factor to amplify or to attenuate the NOx index
 * output. Allowed values are in range 1..1000. The default value is 230.
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sen5x_set_nox_algorithm_tuning_parameters(
    int16_t index_offset, int16_t learning_time_offset_hours,
    int16_t learning_time_gain_hours, int16_t gating_max_duration_minutes,
    int16_t std_initial, int16_t gain_factor);

/**
 * sen5x_get_nox_algorithm_tuning_parameters() - Gets the currently set tuning
 * parameters of the NOx algorithm.
 *
 * Supported sensors: SEN55
 *
 * @param index_offset NOx index representing typical (average) conditions.
 *
 * @param learning_time_offset_hours Time constant to estimate the NOx algorithm
 * offset from the history in hours. Past events will be forgotten after about
 * twice the learning time.
 *
 * @param learning_time_gain_hours The time constant to estimate the NOx
 * algorithm gain from the history has no impact for NOx. This parameter is
 * still in place for consistency reasons with the VOC tuning parameters
 * command.
 *
 * @param gating_max_duration_minutes Maximum duration of gating in minutes
 * (freeze of estimator during high NOx index signal). Zero disables the gating.
 *
 * @param std_initial The initial estimate for standard deviation has no impact
 * for NOx. This parameter is still in place for consistency reasons with the
 * VOC tuning parameters command.
 *
 * @param gain_factor Gain factor to amplify or to attenuate the NOx index
 * output.
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sen5x_get_nox_algorithm_tuning_parameters(
    int16_t* index_offset, int16_t* learning_time_offset_hours,
    int16_t* learning_time_gain_hours, int16_t* gating_max_duration_minutes,
    int16_t* std_initial, int16_t* gain_factor);

/**
 * sen5x_set_rht_acceleration_mode() - Sets the RH/T acceleration mode.
 *
 * Supported sensors: SEN54, SEN55
 *
 * @note This parameter can be changed in any state of the device (and the
 * getter immediately returns the new value), but it is applied only the next
 * time starting a measurement, i.e. when sending a \"Start Measurement\"
 * command. So the parameter needs to be set *before* a new measurement is
 * started.
 *
 * @param mode The new RH/T acceleration mode.
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sen5x_set_rht_acceleration_mode(uint16_t mode);

/**
 * sen5x_get_rht_acceleration_mode() - Gets the RH/T acceleration mode.
 *
 * Supported sensors: SEN54, SEN55
 *
 * @param mode The current RH/T acceleration mode.
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sen5x_get_rht_acceleration_mode(uint16_t* mode);

/**
 * sen5x_set_voc_algorithm_state() - Sets the VOC algorithm state previously
 * received with the \"Get VOC Algorithm State\" command.
 *
 * Supported sensors: SEN54, SEN55
 *
 * @note This command is only available in idle mode and the state will be
 * applied only once when starting the next measurement. Any further
 * measurements (i.e. when stopping and restarting the measure mode) will reset
 * the state to initial values. In measure mode, this command has no effect.
 *
 * @param state VOC algorithm state to restore.
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sen5x_set_voc_algorithm_state(const uint8_t* state, uint8_t state_size);

/**
 * sen5x_get_voc_algorithm_state() - Gets the current VOC algorithm state. This
 * data can be used to restore the state with the \"Set VOC Algorithm State\"
 * command after a short power cycle or device reset.
 *
 * This command can be used either in measure mode or in idle mode
 * (which will then return the state at the time when the measurement
 * was stopped). In measure mode, the state can be read each measure
 * interval to always have the latest state available, even in case of
 * a sudden power loss.
 *
 * Supported sensors: SEN54, SEN55
 *
 * @param state Current VOC algorithm state.
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sen5x_get_voc_algorithm_state(uint8_t* state, uint8_t state_size);

/**
 * sen5x_set_fan_auto_cleaning_interval() - Sets the fan auto cleaning interval
 * for the device.
 *
 * @param interval Fan auto cleaning interval [s]. Set to zero to disable auto
 * cleaning.
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sen5x_set_fan_auto_cleaning_interval(uint32_t interval);

/**
 * sen5x_get_fan_auto_cleaning_interval() - Gets the fan auto cleaning interval
 * from the device.
 *
 * @param interval Fan auto cleaning interval [s]. Zero means auto cleaning is
 * disabled.
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sen5x_get_fan_auto_cleaning_interval(uint32_t* interval);

/**
 * sen5x_get_product_name() - Gets the product name from the device.
 *
 * @param product_name Null-terminated ASCII string containing the product name.
 * Up to 32 characters can be read from the device.
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sen5x_get_product_name(unsigned char* product_name,
                               uint8_t product_name_size);

/**
 * sen5x_get_serial_number() - Gets the serial number from the device.
 *
 * @param serial_number Null-terminated ASCII string containing the serial
 * number. Up to 32 characters can be read from the device.
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sen5x_get_serial_number(unsigned char* serial_number,
                                uint8_t serial_number_size);

/**
 * sen5x_get_version() - Gets the version information for the hardware, firmware
 * and communication protocol.
 *
 * @param firmware_major Firmware major version number.
 *
 * @param firmware_minor Firmware minor version number.
 *
 * @param firmware_debug Firmware debug state. If the debug state is set, the
 * firmware is in development.
 *
 * @param hardware_major Hardware major version number.
 *
 * @param hardware_minor Hardware minor version number.
 *
 * @param protocol_major Protocol major version number.
 *
 * @param protocol_minor Protocol minor version number.
 *
 * @param padding Padding byte, ignore this.
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sen5x_get_version(uint8_t* firmware_major, uint8_t* firmware_minor,
                          bool* firmware_debug, uint8_t* hardware_major,
                          uint8_t* hardware_minor, uint8_t* protocol_major,
                          uint8_t* protocol_minor);

/**
 * sen5x_read_device_status() - Reads the current device status.

 * Use this command to get detailed information about the device status.
 * The device status is encoded in flags. Each device status flag
 * represents a single bit in a 32-bit integer value. If more than one
 * error is present, the device status register value is the sum of the
 * corresponding flag values. For details about the available flags,
 * refer to the device status flags documentation.
 *
 * @note The status flags of type \"Error\" are sticky, i.e. they are not
 * cleared automatically even if the error condition no longer exists. So they
 * can only be cleared manually with the command 0xD210 \"Read And Clear Device
 * Status\" or with a device reset. All other flags are not sticky, i.e. they
 * are cleared automatically if the trigger condition disappears.
 *
 * @param device_status Device status (32 flags as an integer value). For
 * details, please refer to the device status flags documentation.
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sen5x_read_device_status(uint32_t* device_status);

/**
 * sen5x_read_and_clear_device_status() - Reads the current device status (like
 * command 0xD206 \"Read Device Status\") and afterwards clears all flags.
 *
 * @param device_status Device status (32 flags as an integer value) **before**
 * clearing it. For details, please refer to the device status flags
 * documentation.
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sen5x_read_and_clear_device_status(uint32_t* device_status);

/**
 * sen5x_device_reset() - Executes a reset on the device. This has the same
 * effect as a power cycle.
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sen5x_device_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* SEN5X_I2C_H */