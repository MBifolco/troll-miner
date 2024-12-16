#include "asic.h"
#include "esp_log.h"
#include "fayksic.h"
#include "job.h"
#include "queue_handles.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

static const char *TAG = "ASIC";

void asic_task(void *pvParameters) {
    ESP_LOGI(TAG, "Asic task started");

    uint8_t block_header[BLOCK_HEADER_SIZE];

    while (true) {
        // Wait to receive a message from the stratum_to_job_queue
        if (xQueueReceive(work_to_asic_queue, &block_header, portMAX_DELAY) == pdPASS) {
            ESP_LOGI(TAG, "Received work from queue: %p", &block_header);

            print_hex(block_header, BLOCK_HEADER_SIZE, BLOCK_HEADER_SIZE * 2, "ASIC block header: ");
            // send work to asic
            send_work(block_header);
        } else {
            ESP_LOGW(TAG, "Failed to receive work from the queue");
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }
        // Yield to let other tasks run
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
