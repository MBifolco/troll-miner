#include "asic.h"
#include "esp_log.h"
#include "job.h"
#include "mbedtls/sha256.h"
#include "stats.h"
#include "stratum.h"
#include "utils.h"
#include <stdint.h>

static const char *TAG = "FAYKSIC:RESULT";

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
     *
     * Also - these should only be run when a share is accepted
     */
    // calc_hash_rate();

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

    /**
     * TODO: when truediffone is not scaled down for ease, relative diff is always 0?
     */
    uint64_t relative_diff = numerator / denominator;
    update_stats(relative_diff);
    struct scaled_value v = scale_value(relative_diff);
    ESP_LOGI(TAG, "Diff result: %.1f%c/%lu", v.v, v.suffix, STRATUM.session.difficulty);
    print_stats();
}