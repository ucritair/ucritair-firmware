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
    MSG_TYPE_WIFI_CONNECT = 0x51,
    MSG_TYPE_FIRMWARE_VERSION_QUERY = 0x5F,
    MSG_TYPE_WIFI_SCAN_REQUEST = 0x52,
    MSG_TYPE_REBOOT_TO_BOOTLOADER = 0x53,
    MSG_TYPE_ZKP_AUTHENTICATE = 0x54,
} host_to_device_msg_type_t;

// Sent from Device to Host
typedef enum {
    MSG_TYPE_PROTOCOL_VERSION_RESPONSE = 0x00,
    MSG_TYPE_SENSOR_DATA_ACK = 0x70,
    MSG_TYPE_WIFI_CONNECT_ACK = 0x51,
    MSG_TYPE_FIRMWARE_VERSION_RESPONSE = 0x5F,
    MSG_TYPE_WIFI_SCAN_RESPONSE = 0x52,
    MSG_TYPE_REBOOT_TO_BOOTLOADER_ACK = 0x53,
    MSG_TYPE_ZKP_AUTHENTICATE_RESPONSE = 0x54,
    MSG_TYPE_ZKP_AUTH_STATUS = 0x55,  // Unsolicited status updates during auth
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

// Payload for WiFi connect
#define MAX_SSID_LEN 32
#define MAX_PASSWORD_LEN 64

// WiFi auth modes (matches wifi_ap_record_t auth_mode)
#define WIFI_AUTH_OPEN          0
#define WIFI_AUTH_WEP           1
#define WIFI_AUTH_WPA           2
#define WIFI_AUTH_WPA2          3
#define WIFI_AUTH_WPA_WPA2      4

typedef struct __attribute__((__packed__)) {
    char ssid[MAX_SSID_LEN];
    char password[MAX_PASSWORD_LEN];
    uint8_t auth_mode;  // See WIFI_AUTH_* constants above
} msg_payload_wifi_connect_t;

// Payload for WiFi scan results
#define MAX_SCAN_RESULTS 10

typedef struct __attribute__((__packed__)) {
    char ssid[MAX_SSID_LEN];
    uint8_t bssid[6];
    int8_t rssi;           // Signal strength in dBm
    uint8_t channel;
    uint8_t auth_mode;     // 0=Open, 1=WEP, 2=WPA, 3=WPA2, 4=WPA/WPA2
} wifi_ap_record_t;

typedef struct __attribute__((__packed__)) {
    uint8_t count;         // Number of APs found (up to MAX_SCAN_RESULTS)
    wifi_ap_record_t aps[MAX_SCAN_RESULTS];
} msg_payload_wifi_scan_response_t;

// Payload for ZKP authentication request
// Empty payload - device uses hardcoded secret and server hostname
typedef struct __attribute__((__packed__)) {
    // No fields - command is stateless
} msg_payload_zkp_authenticate_request_t;

// Payload for ZKP authentication response
#define MAX_ACCESS_TOKEN_LEN 512
#define MAX_TIMESTAMP_LEN 32

typedef struct __attribute__((__packed__)) {
    uint8_t success;       // 1 if authentication successful, 0 if failed
    char access_token[MAX_ACCESS_TOKEN_LEN];
    char expires_at[MAX_TIMESTAMP_LEN];
} msg_payload_zkp_authenticate_response_t;

// Payload for ZKP authentication status updates (unsolicited)
#define MAX_STATUS_MESSAGE_LEN 128

// Stage definitions
#define ZKP_STAGE_NONCE 1
#define ZKP_STAGE_PARENT 2
#define ZKP_STAGE_WITNESS 3
#define ZKP_STAGE_PROOF 4
#define ZKP_STAGE_VERIFY 5

typedef struct __attribute__((__packed__)) {
    uint8_t stage;         // Current stage (1=nonce, 2=parent, 3=witness, 4=proof, 5=verify)
    uint8_t progress;      // Progress percentage (0-100)
    char message[MAX_STATUS_MESSAGE_LEN];  // Human-readable status message
} msg_payload_zkp_auth_status_t;

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
 * Connect RP2350 to WiFi network
 * @param ssid WiFi SSID (max 32 chars)
 * @param password WiFi password (max 64 chars)
 * @param auth_mode WiFi security type (see WIFI_AUTH_* constants)
 * @param timeout_ms Timeout in milliseconds
 * @return true if connection successful, false if timeout or failed
 */
bool rp2350_wifi_connect(const char *ssid, const char *password, uint8_t auth_mode, uint32_t timeout_ms);

/**
 * Send sensor data to RP2350
 * @param sensor_value Sensor value to send
 * @param timeout_ms Timeout in milliseconds
 * @return true if ACK received, false if timeout
 */
bool rp2350_send_sensor_data(uint32_t sensor_value, uint32_t timeout_ms);

/**
 * Request WiFi scan from RP2350
 * @param results Pointer to store scan results
 * @param timeout_ms Timeout in milliseconds
 * @return true if successful, false if timeout
 */
bool rp2350_wifi_scan(msg_payload_wifi_scan_response_t *results, uint32_t timeout_ms);

/**
 * Reboot RP2350 into USB bootloader mode for firmware updates
 * @param timeout_ms Timeout in milliseconds
 * @return true if ACK received (RP2350 will reboot after ACK), false if timeout
 */
bool rp2350_reboot_to_bootloader(uint32_t timeout_ms);

/**
 * Perform ZKP authentication with server
 * This is a long-running operation (~10 minutes) that will:
 * 1. Send authentication request to RP2350
 * 2. Receive unsolicited status updates (printed to console)
 * 3. Wait for final authentication response
 *
 * @param response Pointer to store authentication response
 * @param timeout_ms Timeout in milliseconds (should be ~15 minutes = 900000ms)
 * @return true if authentication successful, false if failed or timeout
 */
bool rp2350_zkp_authenticate(msg_payload_zkp_authenticate_response_t *response, uint32_t timeout_ms);

/**
 * Process incoming UART data (call periodically)
 */
void rp2350_ipc_process(void);

#endif // RP2350_IPC_H
