#include "asic.h"
#include "esp_log.h"
#include "job.h"
#include "mbedtls/sha256.h"
#include "stratum.h"
#include "utils.h"
#include <stdint.h>

#define RELATIVE_DIFF_NUMERATOR (((uint64_t)TRUEDIFFONE[0] << 32) | TRUEDIFFONE[1])

static const char *TAG = "FAYKSIC:RESULT";

static void print_relative_diff(uint64_t r) {
    const char *suffixes = "\0KMGTP"; // null for values < 1K
    int suffix_index = 0;
    double scaled_r = (double)r;

    // Scale down the value until it's below 1000, updating the suffix
    while (scaled_r >= 1000 && suffix_index < 5) {
        scaled_r /= 1000.0;
        suffix_index++;
    }

    // Format the value with one decimal point and the suffix
    ESP_LOGI(TAG, "Diff result: %.1f%c/%lu", scaled_r, suffixes[suffix_index], STRATUM.session.difficulty);
}

void parse_result(uint8_t *asic_res_buffer, int buffer_size) {
    struct asic_result asic_result;
    asic_result.nonce = (uint32_t)asic_res_buffer[2];
    asic_result.nonce |= ((uint32_t)asic_res_buffer[3] << 8);
    asic_result.nonce |= ((uint32_t)asic_res_buffer[4] << 16);
    asic_result.nonce |= ((uint32_t)asic_res_buffer[5] << 24);
    ESP_LOGD(TAG, "Nonce: 0x%08lx", asic_result.nonce);

    /**
     * Let's have some fun
     *
     * NOTE: These string conversions, prints, and hash calculations are unncessary I/O and
     * should be removed when we get to prime time - it's expensive
     */
    struct block_header bh = build_block_header(CURRENT_JOB);
    bh.bytes[BLOCK_HEADER_SIZE - 1] = asic_res_buffer[2];
    bh.bytes[BLOCK_HEADER_SIZE - 2] = asic_res_buffer[3];
    bh.bytes[BLOCK_HEADER_SIZE - 3] = asic_res_buffer[4];
    bh.bytes[BLOCK_HEADER_SIZE - 4] = asic_res_buffer[5];
    char block_header_str[BLOCK_HEADER_SIZE * 2];
    bin2hex(bh.bytes, BLOCK_HEADER_SIZE, block_header_str, BLOCK_HEADER_SIZE * 2);
    ESP_LOGD(TAG, "Block header: 0x%s", block_header_str);

    uint8_t h[32];
    uint8_t tmp[32];
    mbedtls_sha256(bh.bytes, BLOCK_HEADER_SIZE, tmp, 0);
    mbedtls_sha256(tmp, HASH_SIZE, h, 0);
    reverse_bytes(h, HASH_SIZE);
    char h_str[HASH_SIZE * 2];
    bin2hex(h, HASH_SIZE, h_str, HASH_SIZE * 2);
    ESP_LOGD(TAG, "Hash: 0x%s", h_str);

    uint64_t numerator = RELATIVE_DIFF_NUMERATOR;
    uint64_t denominator = 0;
    for (uint8_t i = 0; i < 8; i++) {
        denominator = (denominator << 8) | h[i];
    }

    uint64_t relative_diff = numerator / denominator;
    print_relative_diff(relative_diff);
}