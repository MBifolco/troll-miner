#include "job.h"
#include "esp_log.h"
#include "stratum_message.h"
#include "queue_handles.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

static const char *TAG = "JOB";


void job_task(void *pvParameters) {
    ESP_LOGI(TAG, "Job task started");

    mining_notify *mining_notify;

    while (true) {
        // Wait to receive a message from the stratum_to_job_queue
        if (xQueueReceive(stratum_to_job_queue, &mining_notify, portMAX_DELAY) == pdPASS) {
            ESP_LOGI(TAG, "Received job from queue: %p", mining_notify);
        } else {
            ESP_LOGW(TAG, "Failed to receive job from the queue");
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        mining_work *mining_work = malloc(sizeof(mining_work));

        // construct and return a mining_work object

        if (mining_work == NULL) {
            ESP_LOGE(TAG, "Failed to allocate memory for mining_work");
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        if (xQueueSend(work_to_asic_queue, &mining_work, portMAX_DELAY) != pdPASS) {
            printf("Failed to send mining_work to queue!\n");
            free(mining_work); // Free memory if sending fails
        }

        // Yield to let other tasks run
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    // this probably needs to be more in depth 
    free(mining_notify)
}
