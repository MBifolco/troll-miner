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

void log_job(struct job *j) {
    ESP_LOGI(TAG, "version: 0x%lx", j->version);
    print_hex(j->previous_block_hash, HASH_SIZE, 160, "PrevBlockHash: ");
    print_hex(j->merkle_tree_root, HASH_SIZE, 160, "MerkleTreeRoot: ");
    ESP_LOGI(TAG, "time: 0x%lx", j->time);
    ESP_LOGI(TAG, "nbits: 0x%lx", j->nbits);
    ESP_LOGI(TAG, "nonce: 0x%lx", (uint32_t)0);
}

void copy_4bytes_to_block_header(uint8_t *bh, uint32_t n, uint8_t idx) {
    bh[idx] = (n >> 24) & 0xFF; // Most significant byte
    bh[idx + 1] = (n >> 16) & 0xFF;
    bh[idx + 2] = (n >> 8) & 0xFF;
    bh[idx + 3] = n & 0xFF; // Least significant byte
}

struct block_header build_block_header(struct job *j) {
    struct block_header bh;
    uint8_t previous_block_hash_offset = 4;
    uint8_t merkle_tree_root_offest = previous_block_hash_offset + HASH_SIZE;
    uint8_t time_offset = merkle_tree_root_offest + HASH_SIZE;
    uint8_t nbits_offset = time_offset + 4;
    uint8_t nonce_offset = nbits_offset + 4;

    copy_4bytes_to_block_header(bh.bytes, j->version, 0);
    memcpy(bh.bytes + previous_block_hash_offset, j->previous_block_hash, HASH_SIZE);
    memcpy(bh.bytes + merkle_tree_root_offest, j->merkle_tree_root, HASH_SIZE);
    copy_4bytes_to_block_header(bh.bytes, j->time, time_offset);
    copy_4bytes_to_block_header(bh.bytes, j->nbits, nbits_offset);
    copy_4bytes_to_block_header(bh.bytes, (uint32_t)0x0f2b5710, nonce_offset); // TODO: Should start @ 0
    print_hex(bh.bytes, BLOCK_HEADER_SIZE, 160, "Block Header: ");
    return bh;
}

void asic_task(void *pvParameters) {
    ESP_LOGI(TAG, "Asic task started");

    struct job *j;

    while (true) {
        // Wait to receive a message from the stratum_to_job_queue
        if (xQueueReceive(work_to_asic_queue, &j, portMAX_DELAY) == pdPASS) {
            ESP_LOGI(TAG, "Received work from queue: %p", j);
            log_job(j);
            build_block_header(j);

            // send work to asic
            send_work(j);

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
