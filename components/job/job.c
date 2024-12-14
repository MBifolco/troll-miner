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
        if (xQueueReceive(stratum_to_job_queue, &received_job, portMAX_DELAY) == pdPASS) {
            format_mining_notify(received_job, log_buffer, sizeof(log_buffer));
            ESP_LOGI(TAG, "Received job from queue: %p", received_job);
            ESP_LOGI(TAG, "Received job: %s", log_buffer);
            free(received_job->job_id);
            free(received_job->prev_block_hash);
            free(received_job->coinbase_prefix);
            free(received_job->coinbase_suffix);
            free(received_job->merkle_branches);
            free(received_job);
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
     // free mining_notify
    

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

