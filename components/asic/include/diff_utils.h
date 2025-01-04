#ifndef ASIC_DIFF_UTILS_H
#define ASIC_DIFF_UTILS_H

#include <stdint.h>

void split_uint32_to_bytes(uint32_t value, uint8_t *bytes, uint8_t idx);
void generate_target_mask(uint32_t target[2], uint32_t res[2]);
int msb_position_32(uint32_t value);

#endif // ASIC_DIFF_UTILS_H
