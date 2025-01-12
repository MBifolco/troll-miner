#ifndef ASIC_COMPONENT_H
#define ASIC_COMPONENT_H
#include "sdkconfig.h"
#include <stdint.h>

#define WORK_SIZE 82

#ifdef CONFIG_SCALE_TRUEDIFF
static const uint32_t TRUEDIFFONE[8] = {0x00FFFFFF, 0, 0, 0, 0, 0, 0, 0};
#else
static const uint32_t TRUEDIFFONE[8] = {0, 0xFFFF0000, 0, 0, 0, 0, 0, 0};
#endif

/**
 * Data structure to store parsed ASIC results.
 *
 * Result should be a uint8_t[11]
 */
struct asic_result {
    // uint8_t preamble[2];     // r[0] & r[1]
    uint32_t nonce;             // r[2], [3], [4], [5]
    // uint8_t midstate_num;    // r[6]
    // uint8_t job_id;          // r[7]
    // uint16_t version;        // r[8], [9]
    // uint8_t crc;             // r[10]
};

void asic_request_task(void *pvParameters);
void asic_result_listener_task(void *pvParameters);

#endif // ASIC_COMPONENT_H
