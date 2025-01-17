#ifndef STATS_COMPONENT_H
#define STATS_COMPONENT_H
#include "asic.h"
#include <stdint.h>
#include <time.h>

#define RELATIVE_DIFF_NUMERATOR (((uint64_t)TRUEDIFFONE[0] << 32) | TRUEDIFFONE[1])
#define CALC_EMA(alpha, current, ema) ((ema == 0.0) ? current : (alpha * current + (1 - alpha) * ema))

static const double alpha_5s = 2.0 / (5.0 + 1.0);
static const double alpha_1m = 2.0 / (60.0 + 1.0);
static const double alpha_5m = 2.0 / (5 * 60.0 + 1.0);
static const double alpha_15m = 2.0 / (15 * 60.0 + 1.0);

struct scaled_value {
    double v;
    char suffix;
};

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

struct scaled_value scale_value(uint64_t);
void update_stats(uint64_t relative_diff);
void print_stats();

#endif // STATS_COMPONENT_H
