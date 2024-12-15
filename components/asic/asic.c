#include "asic.h"
#include "esp_log.h"
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

void log_job(struct job *j) {
    ESP_LOGI(TAG, "0x%lx", j->version);
    print_hex(j->previous_block_hash, HASH_SIZE, 160, "PrevBlockHash: ");
    print_hex(j->merkle_tree_root, HASH_SIZE, 160, "MerkleTreeRoot: ");
    ESP_LOGI(TAG, "0x%lx", j->time);
    ESP_LOGI(TAG, "0x%lx", j->nbits);
    ESP_LOGI(TAG, "0x%lx", (uint32_t)0);
}

void asic_task(void *pvParameters) {
    ESP_LOGI(TAG, "Asic task started");

    struct job *j;

    while (true) {
        // Wait to receive a message from the stratum_to_job_queue
        if (xQueueReceive(work_to_asic_queue, &j, portMAX_DELAY) == pdPASS) {
            ESP_LOGI(TAG, "Received work from queue: %p", j);
            log_job(j);
            free(j->extranonce2);
            free(j);
        } else {
            ESP_LOGW(TAG, "Failed to receive work from the queue");
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }
        // Yield to let other tasks run
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
