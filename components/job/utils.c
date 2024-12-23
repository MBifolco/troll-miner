#include "utils.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "job.h"
#include "mbedtls/sha256.h"
#include "queue_handles.h"
#include "stratum_message.h"
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
    bool ret = false;
    uint8_t *coinbase_prefix = coinbase_section_to_bytes(notify->coinbase_prefix);
    if (coinbase_prefix == NULL) {
        return false;
    }
    uint8_t *coinbase_suffix = coinbase_section_to_bytes(notify->coinbase_suffix);
    if (coinbase_suffix == NULL) {
        goto free_from_prefix;
    }

    size_t extranonce1_hex_len = strlen(extranonce1) / 2;
    uint8_t *extranonce1_hex = malloc(extranonce1_hex_len);
    if (extranonce1_hex == NULL) {
        goto free_from_suffix;
    }

    size_t res = hex2bin(extranonce1, extranonce1_hex, extranonce1_hex_len);
    if (res != extranonce1_hex_len) {
        goto free_from_extranonce1;
    }

    uint8_t extranonce1_offset = strlen(notify->coinbase_prefix) / 2;
    uint8_t extranonce2_offset = extranonce1_offset + extranonce1_hex_len;
    uint8_t cb_suffix_offset = extranonce2_offset + j->extranonce2_len;
    uint8_t cb_tx_len = cb_suffix_offset + strlen(notify->coinbase_suffix) / 2;
    uint8_t *cb_tx = malloc(cb_tx_len);
    if (cb_tx == NULL) {
        goto free_from_extranonce1;
    }

    memcpy(cb_tx, coinbase_prefix, strlen(notify->coinbase_prefix) / 2);
    memcpy(cb_tx + extranonce1_offset, extranonce1_hex, strlen(extranonce1) / 2);
    memcpy(cb_tx + extranonce2_offset, j->extranonce2, j->extranonce2_len);
    memcpy(cb_tx + cb_suffix_offset, coinbase_suffix, strlen(notify->coinbase_suffix) / 2);
    print_hex(cb_tx, cb_tx_len, cb_tx_len, "Raw cb tx: ");

    uint8_t h[32];
    mbedtls_sha256(cb_tx, cb_tx_len, h, 0);
    mbedtls_sha256(h, HASH_SIZE, cb_tx_id, 0);
    ret = true;

    free(cb_tx);
free_from_extranonce1:
    free(extranonce1_hex);
free_from_suffix:
    free(coinbase_suffix);
free_from_prefix:
    free(coinbase_prefix);
    return ret;
}

struct job *construct_job(mining_notify *notify) {
    struct job *j = malloc(sizeof(struct job));
    if (j == NULL) {
        ESP_LOGE(TAG, "job malloc failed");
        return NULL;
    }

    memcpy(j->previous_block_hash, notify->prev_block_hash, HASH_SIZE);
    j->nbits = notify->nbits;
    j->version = notify->version;
    j->time = notify->time;
    j->nonce = 0;

    j->extranonce2_len = 2; // TODO: This comes from mining.subscribe
    j->extranonce2 = malloc(j->extranonce2_len);
    if (j->extranonce2 == NULL) {
        ESP_LOGE(TAG, "extranonce2 allocation failed");
        free(j);
        return NULL;
    }
    // TODO: Doesn't start at 0x0602
    j->extranonce2[0] = 0x06;
    j->extranonce2[1] = 0x02;

    char *extranonce1 = "02\0"; // TODO: This comes from mining.subscribe
    uint8_t coinbase_tx_id[HASH_SIZE];
    bool res = build_coinbase_tx_id(coinbase_tx_id, notify, extranonce1, j);
    if (res == false) {
        free(j->extranonce2);
        free(j);
        return NULL;
    }

    print_hex(coinbase_tx_id, HASH_SIZE, HASH_SIZE, "Coinbase tx ID: ");
    ESP_LOGI(TAG, "Number of merkle branches: %d", notify->n_merkle_branches);

    uint8_t concatenated_branches[HASH_SIZE * 2];
    uint8_t tmp[HASH_SIZE];
    memcpy(concatenated_branches, coinbase_tx_id, HASH_SIZE);
    for (int i = 0; i < notify->n_merkle_branches; ++i) {
        print_hex(notify->merkle_branches + HASH_SIZE * i, HASH_SIZE, 160, "merkle_branch: ");
        memcpy(concatenated_branches + HASH_SIZE, notify->merkle_branches + HASH_SIZE * i, HASH_SIZE);
        mbedtls_sha256(concatenated_branches, HASH_SIZE * 2, tmp, 0);
        mbedtls_sha256(tmp, HASH_SIZE, j->merkle_tree_root, 0);
        // Overwrite the first 32 bytes with the newly computed "merkle_tree_root" for the next round, if necessary
        memcpy(concatenated_branches, j->merkle_tree_root, HASH_SIZE);
    }
    print_hex(j->merkle_tree_root, HASH_SIZE, HASH_SIZE, "Merkle Tree Root: ");

    return j;
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

void free_notify(mining_notify *notify) {
    free(notify->job_id);
    free(notify->coinbase_prefix);
    free(notify->coinbase_suffix);
    free(notify->merkle_branches);
    free(notify);
}