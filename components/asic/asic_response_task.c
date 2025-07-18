#include "asic_response_task.h"
#include "esp_log.h"
#include "bm1366.h"
#include "job.h"
#include "queue_handles.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "serial.h"
#include "driver/uart.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#define BUF_SIZE 2048       // Buffer size for incoming data
static const char *TAG = "ASIC_RESPONSE_TASK";

void asic_response_task(void *pvParameters) {
    ESP_LOGI(TAG, "Asic response task started");

    uint8_t *data = (uint8_t *)malloc(BUF_SIZE); // Allocate buffer for incoming data
    if (data == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for UART buffer");
        vTaskDelete(NULL); // Terminate task if allocation fails
    }

    while (true) {
        // Read data from UART1
        //ESP_LOGI(TAG, "Reading data from UART1");
        int len = uart_read_bytes(UART_NUM_1, data, BUF_SIZE - 1, pdMS_TO_TICKS(1000)); // 1-second timeout

        if (len > 0) {
            ESP_LOGI(TAG, "Received %d bytes", len);
            for (int i = 0; i < len; i++) {
                printf("%02X ", data[i]); // Print each byte in hex
            }
            printf("\n");
        } else {
            ESP_LOGD(TAG, "No data received within timeout");
        }
         // Yield to let other tasks run
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    free(data); // Clean up the buffer (though this point won't be reached in an infinite loop)
    vTaskDelete(NULL);
}
