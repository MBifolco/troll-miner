#include "asic.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "fayksic.h"
#include "freertos/FreeRTOS.h"
#include "utils.h"
#include <string.h>

/**
 * Buffer for reading ASIC result from serial
 *
 * NOTE: The BM_1366/1368/1370 return 11 bytes.
 * But the BM_1397 returns 9 bytes.
 */
static uint8_t BUFF_SIZE = 11;
static const char *TAG = "ASIC_RESULT_LISTENER";

void asic_result_listener_task(void *pvParameters) {
    ESP_LOGI(TAG, "Task started");
    uint8_t buf[BUFF_SIZE];

    while (1) {
        int bytes_read = uart_read_bytes(UART_NUM_1, buf, BUFF_SIZE, 10000 / portTICK_PERIOD_MS);
        if (bytes_read < 0) {
            ESP_LOGE(TAG, "UART error reading data");
        } else if (bytes_read == 0) {
            // TODO: Too many timeouts could indicate the the ASIC is no longer functioning correctly?
            ESP_LOGD(TAG, "UART timed out - no data received");
        } else if (bytes_read != 11) {
            ESP_LOGE(TAG, "Expected to receive 11 bytes, got %d", bytes_read);
        } else if (buf[0] != 0xAA || buf[1] != 0x55) {
            ESP_LOGE(TAG, "Invalid preamble: 0x%02x%02x", buf[0], buf[1]);
        } else {
            char h[BUFF_SIZE * 2];
            to_hex_string(buf, h, bytes_read);

            ESP_LOGD(TAG, "Received %d bytes: %s", bytes_read, h);

            parse_result(buf, BUFF_SIZE);

            // Clear out the memory after we've used it
            memset(h, 0, BUFF_SIZE * 2);
            memset(buf, 0, BUFF_SIZE);
        }
    }
}