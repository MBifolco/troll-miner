#ifndef STRATUM_MESSAGE_COMPONENT_H
#define STRATUM_MESSAGE_COMPONENT_H
#include "cJSON.h"
#include <stdbool.h>
#include <stdint.h>

#define MAX_MERKLE_BRANCHES 32
#define HASH_SIZE 32

typedef enum { MINING_NOTIFY, MINING_SET_DIFFICULTY, MINING_SET_VERSION_MASK } stratum_server_request_method;

typedef struct {
    char *job_id;                // The job ID for the job being sent in this message.
    uint8_t prev_block_hash[32]; // The hex-encoded previous block hash.
    char *coinbase_prefix;       // The hex-encoded prefix of the coinbase transaction (to precede extra nonce 2).
    char *coinbase_suffix;       // The hex-encoded suffix of the coinbase transaction (to follow extra nonce 2).
    uint8_t n_merkle_branches;   // Number of merkle branches
    uint8_t
        *merkle_branches; // A JSON array containing the hex-encoded hashes needed to compute the merkle root. See Merkle Tree Hash Array.
    uint32_t version;     // The hex-encoded block version.
    uint32_t nbits;       // The hex-encoded network difficulty required for the block.
    uint32_t time;        // The hex-encoded current time for the block.
} mining_notify;

void check_queue_items();
void process_message(const char *message);
void parse_message(const char *message_json);
void parse_request(cJSON *method_json);
void parse_response(cJSON *result_json);

#endif // STRATUM_MESSAGE_COMPONENT_H
