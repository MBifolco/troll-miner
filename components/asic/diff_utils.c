#include "diff_utils.h"
#include "esp_log.h"

void split_uint32_to_bytes(uint32_t value, uint8_t *bytes, uint8_t idx) {
    bytes[idx + 0] = (value >> 24) & 0xFF; // Extract the most significant byte
    bytes[idx + 1] = (value >> 16) & 0xFF; // Extract the second most significant byte
    bytes[idx + 2] = (value >> 8) & 0xFF;  // Extract the second least significant byte
    bytes[idx + 3] = value & 0xFF;         // Extract the least significant byte
}

int msb_position_32(uint32_t value) {
    if (value == 0)
        return -1; // No MSB in 0

    int pos = 0;
    while (value >>= 1) {
        pos++;
    }
    return pos;
}

void generate_target_mask(uint32_t input[2], uint32_t result[2]) {
    // Must be signed ints, to get that -1 result!
    // High is input[0], because bits are in descending order (MSB is at [0])
    int msb_pos_high = msb_position_32(input[0]);
    int msb_pos_low = msb_position_32(input[1]);
    int msb_position = msb_pos_high >= 0 ? msb_pos_high + 32 : msb_pos_low;
    ESP_LOGI("DIFF_UTILS", "mbs_pos_high: %d", msb_pos_high);
    ESP_LOGI("DIFF_UTILS", "msb_pos_low: %d", msb_pos_low);
    ESP_LOGI("DIFF_UTILS", "msb_position: %d", msb_position);

    if (msb_position == -1) { // Input is 0
        result[0] = 0;
        result[1] = 0;
        return;
    }

    if (msb_position < 32) {
        result[1] = 0;
        result[0] = (1U << (msb_position + 1)) - 1;
        if (msb_position == 31) {
            result[0]--;
        }
    } else {
        result[1] = (1U << ((msb_position - 32) + 1)) - 1;
        result[0] = 0xFFFFFFFF;
    }
}