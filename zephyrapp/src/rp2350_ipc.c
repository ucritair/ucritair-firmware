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

// Message queue for deferred status printing (avoid printk from ISR)
K_MSGQ_DEFINE(zkp_status_queue, sizeof(msg_payload_zkp_auth_status_t), 20, 4);

// Response tracking
static struct {
    bool waiting;
    bool received;
    uint32_t version;
    int64_t timeout_abs;
    uint8_t debug_buf[512];  // Increased for WiFi scan debugging
    int debug_count;
    msg_payload_wifi_scan_response_t *scan_results;
    msg_payload_zkp_authenticate_response_t *zkp_response;
} response_state = {0};

// UART interrupt handler
static void uart_isr_handler(const struct device *dev, void *user_data)
{
    if (!uart_irq_update(dev)) {
        return;
    }

    if (uart_irq_rx_ready(dev)) {
        uint8_t c;
        int bytes_read = 0;

        // Read all available bytes
        while (uart_fifo_read(dev, &c, 1) == 1) {
            // Store for debugging
            if (response_state.debug_count < sizeof(response_state.debug_buf)) {
                response_state.debug_buf[response_state.debug_count++] = c;
            }

            // Feed to TinyFrame
            if (tf) {
                TF_AcceptChar(tf, c);
            }
            bytes_read++;
        }

        if (bytes_read > 0) {
            LOG_DBG("UART ISR: Read %d bytes (total: %d)", bytes_read, response_state.debug_count);
        }
    }
}

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
    } else {
    	printk("Invalid version response length: %u\n", msg->len);
    }
    return TF_CLOSE;  // Remove listener after handling response
}

/**
 * Generic ACK listener
 */
static TF_Result ack_listener(TinyFrame *tf_instance, TF_Msg *msg)
{
    response_state.received = true;
    response_state.waiting = false;
    printk("ACK received\n");
    return TF_CLOSE;  // Remove listener after handling response
}

/**
 * WiFi scan response listener
 */
static TF_Result wifi_scan_listener(TinyFrame *tf_instance, TF_Msg *msg)
{
    if (msg->len == sizeof(msg_payload_wifi_scan_response_t)) {
        msg_payload_wifi_scan_response_t *payload = (msg_payload_wifi_scan_response_t *)msg->data;

        if (response_state.scan_results) {
            memcpy(response_state.scan_results, payload, sizeof(msg_payload_wifi_scan_response_t));
        }

        response_state.received = true;
        response_state.waiting = false;
        printk("WiFi scan response received: %d APs\n", payload->count);
    } else {
        printk("Invalid WiFi scan response length: %u (expected %u)\n",
               msg->len, sizeof(msg_payload_wifi_scan_response_t));
    }
    return TF_CLOSE;  // Remove listener after handling response
}

/**
 * ZKP authentication status listener (for unsolicited status updates)
 * This is registered as a TYPE listener to catch MSG_TYPE_ZKP_AUTH_STATUS messages
 * NOTE: Called from ISR context, so we queue the message for deferred printing
 */
static TF_Result zkp_status_listener(TinyFrame *tf_instance, TF_Msg *msg)
{
    if (msg->len == sizeof(msg_payload_zkp_auth_status_t)) {
        msg_payload_zkp_auth_status_t status;
        memcpy(&status, msg->data, sizeof(status));

        // Queue the message for processing in thread context (avoid printk from ISR)
        k_msgq_put(&zkp_status_queue, &status, K_NO_WAIT);
    }
    return TF_STAY;
}

/**
 * ZKP authentication response listener (for final authentication result)
 */
static TF_Result zkp_auth_listener(TinyFrame *tf_instance, TF_Msg *msg)
{
    // Filter out status updates (130 bytes) - only process final response (545 bytes)
    if (msg->len == sizeof(msg_payload_zkp_auth_status_t)) {
        // This is a status update, not the final response - ignore
        // (it will be handled by zkp_status_listener)
        return TF_STAY;  // Keep listening for the actual response
    }

    if (msg->len == sizeof(msg_payload_zkp_authenticate_response_t)) {
        msg_payload_zkp_authenticate_response_t *payload = (msg_payload_zkp_authenticate_response_t *)msg->data;

        if (response_state.zkp_response) {
            memcpy(response_state.zkp_response, payload, sizeof(msg_payload_zkp_authenticate_response_t));
        }

        response_state.received = true;
        response_state.waiting = false;

        if (payload->success) {
            printk("\n*** ZKP AUTHENTICATION SUCCESSFUL ***\n");
            printk("Access Token: %.32s...\n", payload->access_token);
            printk("Expires at:   %s\n", payload->expires_at);
        } else {
            printk("\n*** ZKP AUTHENTICATION FAILED ***\n");
        }
        return TF_CLOSE;  // Close after receiving final response
    } else {
        printk("Invalid ZKP auth response length: %u (expected %u)\n",
               msg->len, sizeof(msg_payload_zkp_authenticate_response_t));
    }
    return TF_CLOSE;  // Close on error
}

// --- Public API ---

void rp2350_ipc_init(void)
{
    if (!device_is_ready(uart0_dev)) {
    	printk("UART0 device not ready\n");
        return;
    }

    // Initialize TinyFrame as SLAVE (opposite of RP2350 which is MASTER)
    tf = TF_Init(TF_SLAVE);
    if (!tf) {
    	printk("Failed to initialize TinyFrame\n");
        return;
    }

    // Register type listener for unsolicited ZKP status messages
    TF_AddTypeListener(tf, MSG_TYPE_ZKP_AUTH_STATUS, zkp_status_listener);

    // Configure UART interrupt
    uart_irq_callback_user_data_set(uart0_dev, uart_isr_handler, NULL);
    uart_irq_rx_enable(uart0_dev);
}

void rp2350_ipc_process(void)
{
    if (!tf) {
        return;
    }

    // Process any queued ZKP status messages
    msg_payload_zkp_auth_status_t status;
    while (k_msgq_get(&zkp_status_queue, &status, K_NO_WAIT) == 0) {
        // Stage names for display
        const char *stage_names[] = {
            "Unknown",   // 0
            "Nonce",     // 1
            "Parent",    // 2
            "Witness",   // 3
            "Proof",     // 4
            "Verify"     // 5
        };
        const char *stage_name = (status.stage <= 5) ? stage_names[status.stage] : "Unknown";

        // Color-coded prefix based on progress
        const char *prefix;
        if (status.progress == 0 && strstr(status.message, "failed") == NULL) {
            prefix = "⏳";  // Starting
        } else if (status.progress == 100) {
            prefix = "✓";   // Complete
        } else {
            prefix = "◐";   // In progress
        }

        printk("%s [Stage %u: %s] %3u%% - %s\n",
               prefix, status.stage, stage_name, status.progress, status.message);
    }

    // Check for timeout (UART data is now handled by interrupt)
    if (response_state.waiting && k_uptime_get() >= response_state.timeout_abs) {
    	printk("Response timeout\n");
    	printk("Total bytes received: %d (expected ~420 for WiFi scan)\n", response_state.debug_count);
    	// Print first 50 bytes for debugging
    	int print_count = (response_state.debug_count < 50) ? response_state.debug_count : 50;
    	for (int i = 0; i < print_count; i++) {
    		printk("  [%d]: 0x%02X\n", i, response_state.debug_buf[i]);
    	}
    	if (response_state.debug_count > 50) {
    		printk("  ... (%d more bytes)\n", response_state.debug_count - 50);
    	}
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
    response_state.debug_count = 0;

    // Send query
    printk("Querying firmware version...\n");
    if (!TF_QuerySimple(tf, MSG_TYPE_FIRMWARE_VERSION_QUERY, NULL, 0, version_listener, NULL, 0)) {
        LOG_ERR("Failed to send firmware version query");
        response_state.waiting = false;
        return false;
    }

    // Wait for response (data arrives via interrupt)
    while (response_state.waiting) {
        rp2350_ipc_process();  // Check for timeout
        k_msleep(10);
    }

    if (response_state.received) {
        *major = (response_state.version >> 24) & 0xFF;
        *minor = (response_state.version >> 16) & 0xFF;
        *patch = response_state.version & 0xFFFF;
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
    response_state.debug_count = 0;

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

bool rp2350_wifi_connect(const char *ssid, const char *password, uint8_t auth_mode, uint32_t timeout_ms)
{
    if (!tf || !ssid || !password) {
        return false;
    }

    msg_payload_wifi_connect_t payload = {0};
    strncpy(payload.ssid, ssid, MAX_SSID_LEN);
    strncpy(payload.password, password, MAX_PASSWORD_LEN);
    payload.auth_mode = auth_mode;

    // Reset response state
    response_state.waiting = true;
    response_state.received = false;
    response_state.timeout_abs = k_uptime_get() + timeout_ms;
    response_state.debug_count = 0;

    // Send WiFi connect request
    printk("Sending WiFi connect request (SSID: %s, Auth: %u)...\n", ssid, auth_mode);
    if (!TF_QuerySimple(tf, MSG_TYPE_WIFI_CONNECT, (const uint8_t *)&payload, sizeof(payload), ack_listener, NULL, 0)) {
        LOG_ERR("Failed to send WiFi connect request");
        response_state.waiting = false;
        return false;
    }

    // Wait for ACK with processing (WiFi connection can take 30+ seconds)
    while (response_state.waiting) {
        rp2350_ipc_process();
        k_msleep(100);
    }

    if (response_state.received) {
        printk("WiFi connection successful!\n");
        return true;
    }

    printk("WiFi connection failed or timed out\n");
    return false;
}

bool rp2350_send_sensor_data(uint32_t sensor_value, uint32_t timeout_ms)
{
    // Legacy function - convert single value to array
    uint32_t sensor_values[NUM_SENSORS] = {sensor_value, 0, 0, 0, 0};
    return rp2350_send_sensor_data_array(sensor_values, NUM_SENSORS, timeout_ms);
}

bool rp2350_send_sensor_data_array(uint32_t *sensor_values, uint8_t num_sensors, uint32_t timeout_ms)
{
    if (!tf || !sensor_values || num_sensors != NUM_SENSORS) {
        return false;
    }

    msg_payload_sensor_data_t payload;
    for (int i = 0; i < NUM_SENSORS; i++) {
        payload.sensor_values[i] = sensor_values[i];
    }

    // Reset response state
    response_state.waiting = true;
    response_state.received = false;
    response_state.timeout_abs = k_uptime_get() + timeout_ms;
    response_state.debug_count = 0;

    // Send sensor data
    printk("Sending sensor data: [%u, %u, %u, %u, %u]\n",
           payload.sensor_values[0], payload.sensor_values[1],
           payload.sensor_values[2], payload.sensor_values[3],
           payload.sensor_values[4]);

    if (!TF_QuerySimple(tf, MSG_TYPE_SENSOR_DATA, (const uint8_t *)&payload, sizeof(payload), ack_listener, NULL, 0)) {
        LOG_ERR("Failed to send sensor data");
        response_state.waiting = false;
        return false;
    }

    // Wait for ACK with processing
    // This will take a long time due to TFHE encryption (~60s) + cloud upload
    int64_t start_time = k_uptime_get();
    int64_t last_update = start_time;

    while (response_state.waiting) {
        rp2350_ipc_process();
        k_msleep(100);

        // Print progress every 10 seconds
        int64_t elapsed = k_uptime_get() - start_time;
        if (elapsed - (last_update - start_time) >= 10000) {
            printk("  ... still waiting for ACK (%lld seconds elapsed)\n", elapsed / 1000);
            last_update = k_uptime_get();
        }
    }

    if (response_state.received) {
        int64_t total_time = k_uptime_get() - start_time;
        printk("Sensor data ACK received (%lld seconds)\n", total_time / 1000);
        return true;
    }

    return false;
}

bool rp2350_wifi_scan(msg_payload_wifi_scan_response_t *results, uint32_t timeout_ms)
{
    if (!tf || !results) {
        return false;
    }

    // Reset response state
    response_state.waiting = true;
    response_state.received = false;
    response_state.timeout_abs = k_uptime_get() + timeout_ms;
    response_state.debug_count = 0;
    response_state.scan_results = results;

    // Clear results buffer
    memset(results, 0, sizeof(msg_payload_wifi_scan_response_t));

    // Send scan request
    printk("Requesting WiFi scan from RP2350...\n");
    if (!TF_QuerySimple(tf, MSG_TYPE_WIFI_SCAN_REQUEST, NULL, 0, wifi_scan_listener, NULL, 0)) {
        LOG_ERR("Failed to send WiFi scan request");
        response_state.waiting = false;
        response_state.scan_results = NULL;
        return false;
    }

    // Wait for response (data arrives via interrupt)
    while (response_state.waiting) {
        rp2350_ipc_process();  // Check for timeout
        k_msleep(100);  // WiFi scan takes ~10-20 seconds
    }

    response_state.scan_results = NULL;

    if (response_state.received) {
        printk("WiFi scan complete: %d APs found\n", results->count);
        return true;
    }

    return false;
}

bool rp2350_reboot_to_bootloader(uint32_t timeout_ms)
{
    if (!tf) {
        return false;
    }

    // Reset response state
    response_state.waiting = true;
    response_state.received = false;
    response_state.timeout_abs = k_uptime_get() + timeout_ms;
    response_state.debug_count = 0;

    // Send reboot request
    printk("Requesting RP2350 reboot to bootloader...\n");
    if (!TF_QuerySimple(tf, MSG_TYPE_REBOOT_TO_BOOTLOADER, NULL, 0, ack_listener, NULL, 0)) {
        LOG_ERR("Failed to send reboot to bootloader request");
        response_state.waiting = false;
        return false;
    }

    // Wait for ACK with processing
    while (response_state.waiting) {
        rp2350_ipc_process();
        k_msleep(10);
    }

    if (response_state.received) {
        printk("RP2350 acknowledged reboot request. Device should now be in bootloader mode.\n");
        printk("You can flash new firmware via USB mass storage.\n");
        return true;
    }

    return false;
}

bool rp2350_zkp_authenticate(msg_payload_zkp_authenticate_response_t *response, uint32_t timeout_ms)
{
    if (!tf || !response) {
        return false;
    }

    // Clear response buffer
    memset(response, 0, sizeof(msg_payload_zkp_authenticate_response_t));

    // Reset response state
    response_state.waiting = true;
    response_state.received = false;
    response_state.timeout_abs = k_uptime_get() + timeout_ms;
    response_state.debug_count = 0;
    response_state.zkp_response = response;

    // Print header
    printk("\n");
    printk("========================================\n");
    printk("ZKP AUTHENTICATION\n");
    printk("========================================\n");
    printk("Sending authentication request to RP2350...\n");
    printk("This will take approximately 10-15 minutes.\n");
    printk("\n");
    printk("Status updates:\n");
    printk("----------------------------------------\n");

    // Send ZKP authentication request (empty payload)
    if (!TF_QuerySimple(tf, MSG_TYPE_ZKP_AUTHENTICATE, NULL, 0, zkp_auth_listener, NULL, 0)) {
        LOG_ERR("Failed to send ZKP authentication request");
        response_state.waiting = false;
        response_state.zkp_response = NULL;
        return false;
    }

    // Wait for response with processing
    // Status updates will be dequeued and printed by rp2350_ipc_process()
    int64_t start_time = k_uptime_get();
    int64_t last_heartbeat = 0;
    while (response_state.waiting) {
        rp2350_ipc_process();  // This drains the ZKP status message queue
        k_msleep(50);  // Check every 50ms for responsive status updates

        // Print periodic heartbeat every 60 seconds if no response
        int64_t elapsed = k_uptime_get() - start_time;
        if (elapsed - last_heartbeat >= 60000) {
            printk("[INFO] Still waiting for authentication... (%lld seconds elapsed)\n", elapsed / 1000);
            last_heartbeat = elapsed;
        }
    }

    response_state.zkp_response = NULL;

    int64_t total_time = k_uptime_get() - start_time;

    printk("\n");
    if (response_state.received && response->success) {
        printk("========================================\n");
        printk("AUTHENTICATION SUCCESSFUL\n");
        printk("========================================\n");
        printk("Time taken: %lld seconds (%.1f minutes)\n", total_time / 1000, (float)total_time / 60000.0f);
        printk("========================================\n");
        printk("\n");
        return true;
    } else if (response_state.received && !response->success) {
        printk("========================================\n");
        printk("AUTHENTICATION FAILED\n");
        printk("========================================\n");
        printk("Time taken: %lld seconds\n", total_time / 1000);
        printk("========================================\n");
        printk("\n");
        return false;
    } else {
        printk("========================================\n");
        printk("AUTHENTICATION TIMEOUT\n");
        printk("========================================\n");
        printk("No response after %lld seconds\n", total_time / 1000);
        printk("========================================\n");
        printk("\n");
        return false;
    }
}

