#include "asic.h"
#include "job.h"
#include "esp_log.h"
#include "queue_handles.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

static const char *TAG = "ASIC";


void asic_task(void *pvParameters) {
    ESP_LOGI(TAG, "Asic task started");

    mining_work *received_work;

    while (true) {
        // Wait to receive a message from the stratum_to_job_queue
        if (xQueueReceive(work_to_asic_queue, &received_work, portMAX_DELAY) == pdPASS) {
            ESP_LOGI(TAG, "Received work from queue: %p", received_work);
        } else {
            ESP_LOGW(TAG, "Failed to receive work from the queue");
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }
        // Yield to let other tasks run
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
