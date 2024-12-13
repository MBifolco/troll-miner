#ifndef JOB_COMPONENT_H
#define JOB_COMPONENT_H
#include "stratum_message.h"
#include <stdint.h>
// #include "stratum_message.h"

struct job {
    /**
     * TODO: Say we need to update extranonce2 - how do we rebuild coinbase tx?
     *
     * I'm getting the sense that even more needs to be copied/persisted from mining notify...
     */
    uint8_t merkle_tree_root[32]; // merkle tree root is 32 bytes
    uint8_t previous_block_hash[32];
    uint8_t *extranonce2;
    size_t extranonce2_len;
    uint32_t version;
    uint32_t nbits;
    uint32_t time;
    uint32_t nonce;
};

void job_task(void *pvParameters);
#endif // JOB_COMPONENT_H
