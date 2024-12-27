#include "asic.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "utils.h"
#include <string.h>

static const char *TAG = "ASIC_RESULT_LISTENER";

void asic_result_listener_task(void *pvParameters) {
    ESP_LOGI(TAG, "Task started");
    uint8_t BUFF_SIZE = 6;
    uint8_t buf[BUFF_SIZE];

    while (1) {
        int bytes_read = uart_read_bytes(UART_NUM_1, buf, BUFF_SIZE, 10000 / portTICK_PERIOD_MS);
        if (bytes_read < 0) {
            ESP_LOGE(TAG, "UART error reading data");
        } else if (bytes_read == 0) {
            ESP_LOGI(TAG, "UART timed out - no data received");
        } else {
            char h[BUFF_SIZE * 2];
            to_hex_string(buf, h, bytes_read);
            ESP_LOGI(TAG, "Received %d bytes: %s", bytes_read, h);
            // Clear out the memory after we've used it
            memset(h, 0, BUFF_SIZE * 2);
            memset(buf, 0, BUFF_SIZE);
        }
    }
}