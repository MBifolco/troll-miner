#include "job.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "mbedtls/sha256.h"
#include "queue_handles.h"
#include "stratum_message.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "JOB";

uint8_t *coinbase_section_to_bytes(char *coinbase_str) {
    uint8_t cb_hex_len = strlen(coinbase_str) / 2;
    uint8_t *cb_hex = malloc(cb_hex_len);
    if (cb_hex == NULL) {
        ESP_LOGE(TAG, "cb_hex allocation failed");
        return NULL;
    }

    uint8_t n = hex2bin(coinbase_str, cb_hex, cb_hex_len);
    if (n != cb_hex_len) {
        ESP_LOGE(TAG, "incorrect cb_hex len");
        free(cb_hex);
        return NULL;
    }
    return cb_hex;
}

bool build_coinbase_tx_id(uint8_t *cb_tx_id, mining_notify *notify, char *extranonce1, struct job *j) {
    uint8_t *coinbase_prefix = coinbase_section_to_bytes(notify->coinbase_prefix);
    if (coinbase_prefix == NULL) {
        return false;
    }
    uint8_t *coinbase_suffix = coinbase_section_to_bytes(notify->coinbase_suffix);
    if (coinbase_suffix == NULL) {
        free(coinbase_prefix);
        return false;
    }

    size_t extranonce1_hex_len = strlen(extranonce1) / 2;
    uint8_t *extranonce1_hex = malloc(extranonce1_hex_len);
    if (extranonce1_hex == NULL) {
        free(coinbase_prefix);
        free(coinbase_suffix);
        return false;
    }

    size_t res = hex2bin(extranonce1, extranonce1_hex, extranonce1_hex_len);
    if (res != extranonce1_hex_len) {
        free(coinbase_prefix);
        free(coinbase_suffix);
        free(extranonce1_hex);
        return false;
    }

    uint8_t extranonce1_offset = strlen(notify->coinbase_prefix) / 2;
    uint8_t extranonce2_offset = extranonce1_offset + extranonce1_hex_len;
    uint8_t cb_suffix_offset = extranonce2_offset + j->extranonce2_len;
    uint8_t cb_tx_len = cb_suffix_offset + strlen(notify->coinbase_suffix) / 2;
    uint8_t *cb_tx = malloc(cb_tx_len);
    if (cb_tx == NULL) {
        free(coinbase_prefix);
        free(coinbase_suffix);
        free(extranonce1_hex);
        return false;
    }

    memcpy(cb_tx, coinbase_prefix, strlen(notify->coinbase_prefix) / 2);
    memcpy(cb_tx + extranonce1_offset, extranonce1_hex, strlen(extranonce1) / 2);
    memcpy(cb_tx + extranonce2_offset, j->extranonce2, j->extranonce2_len);
    memcpy(cb_tx + cb_suffix_offset, coinbase_suffix, strlen(notify->coinbase_suffix) / 2);
    print_hex(cb_tx, cb_tx_len, cb_tx_len, "Raw cb tx: ");

    uint8_t h[32];
    mbedtls_sha256(cb_tx, cb_tx_len, h, 0);
    mbedtls_sha256(h, HASH_SIZE, cb_tx_id, 0);

    free(cb_tx);
    free(coinbase_prefix);
    free(coinbase_suffix);
    free(extranonce1_hex);
    return true;
}

struct job *construct_job(mining_notify *notify) {
    struct job *j = malloc(sizeof(struct job)); // TODO: Check malloc succeeded aka j != NULL
    if (j == NULL) {
        ESP_LOGE(TAG, "job allocation failed");
        return NULL;
    }
    memcpy(j->previous_block_hash, notify->prev_block_hash, HASH_SIZE);
    j->nbits = notify->nbits;
    j->version = notify->version;
    j->time = notify->time;
    j->nonce = 0;

    j->extranonce2_len = 2;                      // TODO: This comes from mining.subscribe
    j->extranonce2 = malloc(j->extranonce2_len); // TODO: check malloc res
    if (j->extranonce2 == NULL) {
        ESP_LOGE(TAG, "extranonce2 allocation failed");
        free(j);
        return NULL;
    }
    // TODO: Doesn't start at 0x0602
    j->extranonce2[0] = 0x06;
    j->extranonce2[1] = 0x02;

    char *extranonce1 = "02\0"; // TODO: This comes from mining.subscribe
    uint8_t *coinbase_tx_id = malloc(HASH_SIZE);
    if (coinbase_tx_id == NULL) {
        ESP_LOGE(TAG, "coinbase_tx_id construction failed");
        free(j->extranonce2);
        free(j);
        return NULL;
    }

    bool res = build_coinbase_tx_id(coinbase_tx_id, notify, extranonce1, j); // TODO: check if coinbase_tx_id == NULL
    if (res == false) {
        free(coinbase_tx_id);
        free(j->extranonce2);
        free(j);
        return NULL;
    }

    print_hex(coinbase_tx_id, HASH_SIZE, HASH_SIZE, "Coinbase tx ID: ");

    free(coinbase_tx_id);
    return j;
}

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

void job_task(void *pvParameters) {
    ESP_LOGI(TAG, "Job task started");

    mining_notify *notify;

    while (true) {
        // Wait to receive a message from the stratum_to_job_queue
        if (xQueueReceive(stratum_to_job_queue, &notify, portMAX_DELAY) == pdPASS) {
            log_notify(notify);
            struct job *j = construct_job(notify);

            /**
             * TODO: Double check if this is legit. It was allocated in stratum_message, but gotta make sure
             * a subsequent message can still be handled...
             */
            free(notify);
            free(j); // Probably too soon to frees
        } else {
            ESP_LOGW(TAG, "Failed to receive job from the queue");
        }

        // Yield to let other tasks run
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}