#include "asic.h"
#include "esp_log.h"
#include "job.h"
#include "mbedtls/sha256.h"
#include "stratum.h"
#include "utils.h"
#include <math.h>
#include <stdint.h>
#include <time.h>

#define RELATIVE_DIFF_NUMERATOR (((uint64_t)TRUEDIFFONE[0] << 32) | TRUEDIFFONE[1])
#define CALC_EMA(alpha, current, ema) ((ema == 0.0) ? current : (alpha * current + (1 - alpha) * ema))

static const char *TAG = "FAYKSIC:RESULT";
static const double alpha_5s = 1.0 / (5.0 + 1.0);
static const double alpha_1m = 1.0 / (60.0 + 1.0);
static const double alpha_5m = 1.0 / (5 * 60.0 + 1.0);
static const double alpha_15m = 1.0 / (15 * 60.0 + 1.0);

struct stats {
    uint64_t max_diff;

    /* Hash rates*/
    time_t last_share_time;

    /**
     * Use cgminer pattern of averages
     */
    uint32_t total_shares;
    time_t start_time;
    double uptime;
    double ema_5s;
    double ema_1m;
    double ema_5m;
    double ema_15m;
    double avg_rate;
};

struct stats STATS;

static void calc_hash_rate() {
    time_t current_time = time(NULL);

    // Assumes calc_hash_rate only gets called on successful share
    STATS.total_shares++;

    if (STATS.start_time == 0) {
        STATS.start_time = current_time;
    } else {
        STATS.uptime = difftime(current_time, STATS.start_time);
        STATS.avg_rate = STATS.total_shares / STATS.uptime;
    }

    // If this isn't the first share, calculate elapsed time and current rate
    if (STATS.last_share_time > 0) {
        double elapsed_time = difftime(current_time, STATS.last_share_time);
        if (elapsed_time > 0) {
            double current_rate = 1.0 / elapsed_time; // Shares per second
            STATS.ema_5s = CALC_EMA(alpha_5s, current_rate, STATS.ema_5s);
            STATS.ema_1m = CALC_EMA(alpha_1m, current_rate, STATS.ema_1m);
            STATS.ema_5m = CALC_EMA(alpha_5m, current_rate, STATS.ema_5m);
            STATS.ema_15m = CALC_EMA(alpha_15m, current_rate, STATS.ema_15m);
        }
    }

    STATS.last_share_time = current_time;
}

static void print_stats(uint64_t r) {
    if (r > STATS.max_diff) {
        STATS.max_diff = r;
    }

    const char *suffixes = "\0KMGTP"; // null for values < 1K
    int suffix_index = 0;
    double scaled_r = (double)r;

    // Scale down the value until it's below 1000, updating the suffix
    while (scaled_r >= 1000 && suffix_index < 5) {
        scaled_r /= 1000.0;
        suffix_index++;
    }

    double hash_rate_5s = STATS.ema_5s * STRATUM.session.difficulty;
    double hash_rate_1m = STATS.ema_1m * STRATUM.session.difficulty;
    double hash_rate_5m = STATS.ema_5m * STRATUM.session.difficulty;
    double hash_rate_15m = STATS.ema_15m * STRATUM.session.difficulty;
    double hash_rate_avg = STATS.avg_rate * STRATUM.session.difficulty;
    scaled_r = floor(scaled_r * 10) / 10.0;
    ESP_LOGI(TAG, "Diff result: %.1f%c/%lu -- Max diff(%llu) -- hash rates: %.2f/%.2f/%.2f/%.2f/%.2f -- uptime %.0f", scaled_r,
             suffixes[suffix_index], STRATUM.session.difficulty, STATS.max_diff, hash_rate_5s, hash_rate_1m, hash_rate_5m, hash_rate_15m,
             hash_rate_avg, STATS.uptime);
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
     *
     * Also - these should only be run when a share is accepted
     */
    calc_hash_rate();

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
    print_stats(relative_diff);
}