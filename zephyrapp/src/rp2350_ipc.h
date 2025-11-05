#ifndef RP2350_IPC_H
#define RP2350_IPC_H

#include <stdint.h>
#include <stdbool.h>

// --- Protocol Version (must match RP2350) ---
#define PROTOCOL_VERSION_MAJOR 1
#define PROTOCOL_VERSION_MINOR 0
#define PROTOCOL_VERSION_PATCH 0

// --- Firmware Version ---
#define FIRMWARE_VERSION_MAJOR 1
#define FIRMWARE_VERSION_MINOR 0
#define FIRMWARE_VERSION_PATCH 0

// --- Message Types (must match RP2350) ---
// Sent from Host to Device
typedef enum {
    MSG_TYPE_PROTOCOL_VERSION_QUERY = 0x00,
    MSG_TYPE_SENSOR_DATA = 0x70,
    MSG_TYPE_SET_WIFI_CREDENTIALS = 0x51,
    MSG_TYPE_FIRMWARE_VERSION_QUERY = 0x5F,
} host_to_device_msg_type_t;

// Sent from Device to Host
typedef enum {
    MSG_TYPE_PROTOCOL_VERSION_RESPONSE = 0x00,
    MSG_TYPE_SENSOR_DATA_ACK = 0x70,
    MSG_TYPE_SET_WIFI_CREDENTIALS_ACK = 0x51,
    MSG_TYPE_FIRMWARE_VERSION_RESPONSE = 0x5F,
} device_to_host_msg_type_t;

// --- Message Payloads ---

// Payload for version responses
typedef struct __attribute__((__packed__)) {
    uint32_t version;
} msg_payload_version_response_t;

// Payload for sensor data
typedef struct __attribute__((__packed__)) {
    uint32_t sensor_value;
} msg_payload_sensor_data_t;

// Payload for setting WiFi credentials
#define MAX_SSID_LEN 32
#define MAX_PASSWORD_LEN 64

typedef struct __attribute__((__packed__)) {
    char ssid[MAX_SSID_LEN];
    char password[MAX_PASSWORD_LEN];
} msg_payload_set_wifi_credentials_t;

// --- API Functions ---

/**
 * Initialize RP2350 IPC (UART0 + TinyFrame)
 */
void rp2350_ipc_init(void);

/**
 * Query firmware version from RP2350
 * @param major Pointer to store major version
 * @param minor Pointer to store minor version
 * @param patch Pointer to store patch version
 * @param timeout_ms Timeout in milliseconds
 * @return true if successful, false if timeout
 */
bool rp2350_query_firmware_version(uint8_t *major, uint8_t *minor, uint16_t *patch, uint32_t timeout_ms);

/**
 * Query protocol version from RP2350
 * @param major Pointer to store major version
 * @param minor Pointer to store minor version
 * @param patch Pointer to store patch version
 * @param timeout_ms Timeout in milliseconds
 * @return true if successful, false if timeout
 */
bool rp2350_query_protocol_version(uint8_t *major, uint8_t *minor, uint16_t *patch, uint32_t timeout_ms);

/**
 * Send WiFi credentials to RP2350
 * @param ssid WiFi SSID (max 32 chars)
 * @param password WiFi password (max 64 chars)
 * @param timeout_ms Timeout in milliseconds
 * @return true if ACK received, false if timeout
 */
bool rp2350_set_wifi_credentials(const char *ssid, const char *password, uint32_t timeout_ms);

/**
 * Send sensor data to RP2350
 * @param sensor_value Sensor value to send
 * @param timeout_ms Timeout in milliseconds
 * @return true if ACK received, false if timeout
 */
bool rp2350_send_sensor_data(uint32_t sensor_value, uint32_t timeout_ms);

/**
 * Process incoming UART data (call periodically)
 */
void rp2350_ipc_process(void);

#endif // RP2350_IPC_H
