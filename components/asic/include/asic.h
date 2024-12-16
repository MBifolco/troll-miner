#ifndef ASIC_COMPONENT_H
#define ASIC_COMPONENT_H
#include <stdint.h>

#define BLOCK_HEADER_SIZE 80

struct block_header {
    uint8_t bytes[BLOCK_HEADER_SIZE];
};

void asic_task(void *pvParameters);

#endif // ASIC_COMPONENT_H
