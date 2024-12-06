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

    mining_notify *received_job;
    char log_buffer[512]; // Buffer to hold the formatted string

    while (true) {
        // Wait to receive a message from the stratum_to_job_queue
        if (xQueueReceive(stratum_to_job_queue, &received_job, portMAX_DELAY) == pdPASS) {
            format_mining_notify(received_job, log_buffer, sizeof(log_buffer));
            ESP_LOGI(TAG, "Received job from queue: %p", received_job);
            ESP_LOGI(TAG, "Received job: %s", log_buffer);
        } else {
            ESP_LOGW(TAG, "Failed to receive job from the queue");
        }

        // Yield to let other tasks run
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void format_mining_notify(const mining_notify *job, char *output, size_t output_size) {
    if (job == NULL || output == NULL) {
        snprintf(output, output_size, "Invalid mining_notify object");
        return;
    }

    
    snprintf(output, output_size,
             "Job ID: %s, Prev Block Hash: %s, Coinbase Prefix: %s, "
             "Coinbase Suffix: %s",
             job->job_id,
             job->prev_block_hash,
             job->coinbase_prefix,
             job->coinbase_suffix
    );
}
