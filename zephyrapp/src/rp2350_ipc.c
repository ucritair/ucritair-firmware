#include "rp2350_ipc.h"
#include "TinyFrame.h"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(rp2350_ipc, LOG_LEVEL_INF);

// UART0 device
static const struct device *uart0_dev = DEVICE_DT_GET(DT_NODELABEL(uart0));

// TinyFrame instance
static TinyFrame *tf = NULL;

// Response tracking
static struct {
    bool waiting;
    bool received;
    uint32_t version;
    int64_t timeout_abs;
} response_state = {0};

// --- TinyFrame callbacks ---

/**
 * TinyFrame write implementation - sends data over UART1
 */
void TF_WriteImpl(TinyFrame *tf_instance, const uint8_t *buff, uint32_t len)
{
    if (!device_is_ready(uart0_dev)) {
        LOG_ERR("UART0 not ready");
        return;
    }

    LOG_INF("Sending %u bytes to UART0", len);
    for (uint32_t i = 0; i < len; i++) {
        uart_poll_out(uart0_dev, buff[i]);
    }
}

/**
 * Version response listener
 */
static TF_Result version_listener(TinyFrame *tf_instance, TF_Msg *msg)
{
    if (msg->len == sizeof(msg_payload_version_response_t)) {
        msg_payload_version_response_t *payload = (msg_payload_version_response_t *)msg->data;
        response_state.version = payload->version;
        response_state.received = true;
        response_state.waiting = false;
        LOG_INF("Version response received: 0x%08X", response_state.version);
    } else {
        LOG_ERR("Invalid version response length: %u", msg->len);
    }
    return TF_STAY;
}

/**
 * Generic ACK listener
 */
static TF_Result ack_listener(TinyFrame *tf_instance, TF_Msg *msg)
{
    response_state.received = true;
    response_state.waiting = false;
    LOG_INF("ACK received");
    return TF_STAY;
}

// --- Public API ---

void rp2350_ipc_init(void)
{
    if (!device_is_ready(uart0_dev)) {
        LOG_ERR("UART0 device not ready");
        return;
    }

    // Initialize TinyFrame as SLAVE (opposite of RP2350 which is MASTER)
    tf = TF_Init(TF_SLAVE);
    if (!tf) {
        LOG_ERR("Failed to initialize TinyFrame");
        return;
    }

    LOG_INF("RP2350 IPC initialized on UART0");
}

void rp2350_ipc_process(void)
{
    if (!device_is_ready(uart0_dev) || !tf) {
        return;
    }

    // Read available bytes from UART0 and feed to TinyFrame
    uint8_t c;
    int bytes_read = 0;
    while (uart_poll_in(uart0_dev, &c) == 0) {
        TF_AcceptChar(tf, c);
        bytes_read++;
    }

    if (bytes_read > 0) {
        LOG_INF("Read %d bytes from UART0", bytes_read);
    }

    // Check for timeout
    if (response_state.waiting && k_uptime_get() >= response_state.timeout_abs) {
        LOG_WRN("Response timeout");
        response_state.waiting = false;
        response_state.received = false;
    }
}

bool rp2350_query_firmware_version(uint8_t *major, uint8_t *minor, uint16_t *patch, uint32_t timeout_ms)
{
    if (!tf || !major || !minor || !patch) {
        return false;
    }

    // Reset response state
    response_state.waiting = true;
    response_state.received = false;
    response_state.version = 0;
    response_state.timeout_abs = k_uptime_get() + timeout_ms;

    // Send query
    LOG_INF("Querying firmware version...");
    if (!TF_QuerySimple(tf, MSG_TYPE_FIRMWARE_VERSION_QUERY, NULL, 0, version_listener, NULL, 0)) {
        LOG_ERR("Failed to send firmware version query");
        response_state.waiting = false;
        return false;
    }

    // Wait for response with processing
    while (response_state.waiting) {
        rp2350_ipc_process();
        k_msleep(10);
    }

    if (response_state.received) {
        *major = (response_state.version >> 24) & 0xFF;
        *minor = (response_state.version >> 16) & 0xFF;
        *patch = response_state.version & 0xFFFF;
        LOG_INF("Firmware version: %u.%u.%u", *major, *minor, *patch);
        return true;
    }

    return false;
}

bool rp2350_query_protocol_version(uint8_t *major, uint8_t *minor, uint16_t *patch, uint32_t timeout_ms)
{
    if (!tf || !major || !minor || !patch) {
        return false;
    }

    // Reset response state
    response_state.waiting = true;
    response_state.received = false;
    response_state.version = 0;
    response_state.timeout_abs = k_uptime_get() + timeout_ms;

    // Send query
    LOG_INF("Querying protocol version...");
    if (!TF_QuerySimple(tf, MSG_TYPE_PROTOCOL_VERSION_QUERY, NULL, 0, version_listener, NULL, 0)) {
        LOG_ERR("Failed to send protocol version query");
        response_state.waiting = false;
        return false;
    }

    // Wait for response with processing
    while (response_state.waiting) {
        rp2350_ipc_process();
        k_msleep(10);
    }

    if (response_state.received) {
        *major = (response_state.version >> 24) & 0xFF;
        *minor = (response_state.version >> 16) & 0xFF;
        *patch = response_state.version & 0xFFFF;
        LOG_INF("Protocol version: %u.%u.%u", *major, *minor, *patch);
        return true;
    }

    return false;
}

bool rp2350_set_wifi_credentials(const char *ssid, const char *password, uint32_t timeout_ms)
{
    if (!tf || !ssid || !password) {
        return false;
    }

    msg_payload_set_wifi_credentials_t payload = {0};
    strncpy(payload.ssid, ssid, MAX_SSID_LEN);
    strncpy(payload.password, password, MAX_PASSWORD_LEN);

    // Reset response state
    response_state.waiting = true;
    response_state.received = false;
    response_state.timeout_abs = k_uptime_get() + timeout_ms;

    // Send credentials
    LOG_INF("Sending WiFi credentials...");
    if (!TF_QuerySimple(tf, MSG_TYPE_SET_WIFI_CREDENTIALS, (const uint8_t *)&payload, sizeof(payload), ack_listener, NULL, 0)) {
        LOG_ERR("Failed to send WiFi credentials");
        response_state.waiting = false;
        return false;
    }

    // Wait for ACK with processing
    while (response_state.waiting) {
        rp2350_ipc_process();
        k_msleep(10);
    }

    if (response_state.received) {
        LOG_INF("WiFi credentials ACK received");
        return true;
    }

    return false;
}

bool rp2350_send_sensor_data(uint32_t sensor_value, uint32_t timeout_ms)
{
    if (!tf) {
        return false;
    }

    msg_payload_sensor_data_t payload = {
        .sensor_value = sensor_value
    };

    // Reset response state
    response_state.waiting = true;
    response_state.received = false;
    response_state.timeout_abs = k_uptime_get() + timeout_ms;

    // Send sensor data
    LOG_DBG("Sending sensor data: %u", sensor_value);
    if (!TF_QuerySimple(tf, MSG_TYPE_SENSOR_DATA, (const uint8_t *)&payload, sizeof(payload), ack_listener, NULL, 0)) {
        LOG_ERR("Failed to send sensor data");
        response_state.waiting = false;
        return false;
    }

    // Wait for ACK with processing
    while (response_state.waiting) {
        rp2350_ipc_process();
        k_msleep(10);
    }

    if (response_state.received) {
        LOG_DBG("Sensor data ACK received");
        return true;
    }

    return false;
}
