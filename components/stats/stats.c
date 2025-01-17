#include "stats.h"
#include "esp_log.h"
#include "stratum.h"
#include <math.h>

static const char *TAG = "STATS";
static struct stats STATS;
static const char *SUFFIXES = "\0KMGTP"; // null for values < 1K
static time_t LAST_STATS_UPDATE = 0;

void update_stats(uint64_t relative_diff) {
    if (relative_diff > STATS.max_diff) {
        STATS.max_diff = relative_diff;
    }

    time_t current_time = time(NULL);

    // Assumes update_stats only gets called on successful share
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

struct scaled_value scale_value(uint64_t v) {
    int suffix_index = 0;
    double scaled_v = (double)v;

    // Scale down the value until it's below 1000, updating the suffix
    while (scaled_v >= 1000 && suffix_index < 5) {
        scaled_v /= 1000.0;
        suffix_index++;
    }

    scaled_v = floor(scaled_v * 10) / 10.0;
    struct scaled_value s = {scaled_v, SUFFIXES[suffix_index]};
    return s;
}

void print_stats() {
    time_t now = time(NULL);
    if (now - LAST_STATS_UPDATE < 60) {
        return;
    }

    LAST_STATS_UPDATE = now;
    double hash_rate_5s = STATS.ema_5s * STRATUM.session.difficulty;
    double hash_rate_1m = STATS.ema_1m * STRATUM.session.difficulty;
    double hash_rate_5m = STATS.ema_5m * STRATUM.session.difficulty;
    double hash_rate_15m = STATS.ema_15m * STRATUM.session.difficulty;
    double hash_rate_avg = STATS.avg_rate * STRATUM.session.difficulty;
    ESP_LOGI(TAG, "Max diff(%llu) -- hash rates: %.2f/%.2f/%.2f/%.2f/%.2f -- uptime %.0f", STATS.max_diff, hash_rate_5s, hash_rate_1m,
             hash_rate_5m, hash_rate_15m, hash_rate_avg, STATS.uptime);
}