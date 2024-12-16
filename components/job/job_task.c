#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "job.h"
#include "mbedtls/sha256.h"
#include "queue_handles.h"
#include "stratum_message.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "JOB";

void log_notify(mining_notify *notify) {
    ESP_LOGI(TAG, "Received job from queue: %p", notify);
    ESP_LOGI(TAG, "Job ID: %s", notify->job_id);
    print_hex(notify->prev_block_hash, HASH_SIZE, 160, "PrevBlockHash: ");
    ESP_LOGI(TAG, "Coinbase prefix: %s", notify->coinbase_prefix);
    ESP_LOGI(TAG, "Coinbase suffix: %s", notify->coinbase_suffix);
    ESP_LOGI(TAG, "Version: 0x%lx", notify->version);
    ESP_LOGI(TAG, "nBits: 0x%lx", notify->nbits);
    ESP_LOGI(TAG, "Time: 0x%lx", notify->time);
}

void log_job(struct job *j) {
    ESP_LOGI(TAG, "version: 0x%lx", j->version);
    print_hex(j->previous_block_hash, HASH_SIZE, 160, "PrevBlockHash: ");
    print_hex(j->merkle_tree_root, HASH_SIZE, 160, "MerkleTreeRoot: ");
    ESP_LOGI(TAG, "time: 0x%lx", j->time);
    ESP_LOGI(TAG, "nbits: 0x%lx", j->nbits);
    ESP_LOGI(TAG, "nonce: 0x%lx", (uint32_t)0);
}

void job_task(void *pvParameters) {
    ESP_LOGI(TAG, "Job task started");

    mining_notify *notify;

    while (true) {
        // Wait to receive a message from the stratum_to_job_queue
        if (xQueueReceive(stratum_to_job_queue, &notify, portMAX_DELAY) == pdPASS) {
            log_notify(notify);
            struct job *j = construct_job(notify);
            free_notify(notify);

            if (j == NULL) {
                ESP_LOGE(TAG, "Failed to allocate memory for mining_work");
                vTaskDelay(pdMS_TO_TICKS(10));
                continue;
            }

            log_job(j);
            struct block_header bh = build_block_header(j);
            free(j->extranonce2);
            free(j);

            if (xQueueSend(work_to_asic_queue, &bh.bytes, portMAX_DELAY) != pdPASS) {
                printf("Failed to send mining_work to queue!\n");
            }
        } else {
            ESP_LOGW(TAG, "Failed to receive job from the queue");
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        // Yield to let other tasks run
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    // free mining_notify
}