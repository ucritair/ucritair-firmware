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
    uint8_t debug_buf[512];  // Increased for WiFi scan debugging
    int debug_count;
    msg_payload_wifi_scan_response_t *scan_results;
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
    return TF_STAY;
}

/**
 * Generic ACK listener
 */
static TF_Result ack_listener(TinyFrame *tf_instance, TF_Msg *msg)
{
    response_state.received = true;
    response_state.waiting = false;
    printk("ACK received\n");
    return TF_STAY;
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
    return TF_STAY;
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

    // Configure UART interrupt
    uart_irq_callback_user_data_set(uart0_dev, uart_isr_handler, NULL);
    uart_irq_rx_enable(uart0_dev);
}

void rp2350_ipc_process(void)
{
    if (!tf) {
        return;
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
    response_state.debug_count = 0;

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
    response_state.debug_count = 0;

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

