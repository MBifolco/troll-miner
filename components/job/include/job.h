#ifndef JOB_COMPONENT_H
#define JOB_COMPONENT_H

#include <stdint.h>

typedef struct
{
    char *job_id; //The job ID for the job being sent in this message.
    char *prev_block_hash; //The hex-encoded previous block hash.
    char *coinbase_prefix; //The hex-encoded prefix of the coinbase transaction (to precede extra nonce 2).
    char *coinbase_suffix; //The hex-encoded suffix of the coinbase transaction (to follow extra nonce 2).
    uint8_t *merkle_branches; //A JSON array containing the hex-encoded hashes needed to compute the merkle root. See Merkle Tree Hash Array.
    uint32_t block_version; //The hex-encoded block version.
    uint32_t network_difficulty; //The hex-encoded network difficulty required for the block.
    uint32_t ntime; //The hex-encoded current time for the block.
} mining_notify;

#endif // JOB_COMPONENT_H
