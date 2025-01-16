#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "job.h"
#include "mbedtls/sha256.h"
#include "queue_handles.h"
#include "stratum.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "JOB";
struct job *CURRENT_JOB;

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

    /**
     * TODO: Need a semaphore for CURRENT_JOB. When new jobs come in,
     * we need a way to ensure we're not overwriting state that the
     * asic_result task is in the middle of using.
     */
    CURRENT_JOB = malloc(sizeof(struct job));
    if (CURRENT_JOB == NULL) {
        ESP_LOGE(TAG, "CURRENT_JOB malloc failed");
        return;
    }

    /**
     * TODO: mining.subscribe server response does this allocation
     * TODO: Check this operation was done; this is the (rare) case where we
     * happen to get a mining.notify before a mining.subscribe response - we
     * cannot send work to the ASIC if we don't know the extranonce2 info
     */
    CURRENT_JOB->extranonce2_len = 2; // TODO:
    CURRENT_JOB->extranonce2 = malloc(CURRENT_JOB->extranonce2_len);
    if (CURRENT_JOB->extranonce2 == NULL) {
        ESP_LOGE(TAG, "CURRENT_JOB->extranonce2 allocation failed");
        free(CURRENT_JOB);
        return;
    }

    mining_notify *notify;

    while (true) {
        // Wait to receive a message from the stratum_to_job_queue
        if (xQueueReceive(stratum_to_job_queue, &notify, portMAX_DELAY) == pdPASS) {
            log_notify(notify);
            construct_job(notify, CURRENT_JOB);
            free_notify(notify);

            log_job(CURRENT_JOB);
            struct block_header bh = build_block_header(CURRENT_JOB);

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
    free(CURRENT_JOB);
}