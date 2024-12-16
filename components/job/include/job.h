#ifndef JOB_COMPONENT_H
#define JOB_COMPONENT_H
#include "stratum_message.h"
#include <stdint.h>

#define BLOCK_HEADER_SIZE 80

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

struct block_header {
    uint8_t bytes[BLOCK_HEADER_SIZE];
};

void job_task(void *pvParameters);
uint8_t *coinbase_section_to_bytes(char *coinbase_str);
bool build_coinbase_tx_id(uint8_t *cb_tx_id, mining_notify *notify, char *extranonce1, struct job *j);
struct job *construct_job(mining_notify *notify);
void copy_4bytes_to_block_header(uint8_t *bh, uint32_t n, uint8_t idx);
struct block_header build_block_header(struct job *j);
void free_notify(mining_notify *notify);

#endif // JOB_COMPONENT_H
