#ifndef STRATUM_MESSAGE_COMPONENT_H
#define STRATUM_MESSAGE_COMPONENT_H
#include "cJSON.h"
#include <stdbool.h>
#include <stdint.h>

#define MAX_MERKLE_BRANCHES 32
#define HASH_SIZE 32
#define COINBASE_SIZE 100
#define COINBASE2_SIZE 128

// docs on reqeusts and responses, methods and results
// https://reference.cash/mining/stratum-protocol
typedef enum { MINING_NOTIFY, MINING_SET_DIFFICULTY, MINING_SET_VERSION_MASK, NUM_METHODS } stratum_server_request_method;

// error code mapping
// https://reference.cash/mining/stratum-protocol#error-codes
typedef enum {
    OTHER_UNKNOWN = 20,
    JOB_NOT_FOUND = 21,
    DUPLICATE_SHARE = 22,
    LOW_DIFFICULTY_SHARE = 23,
    UNAUTHRIZED_WORKER = 24,
    NOT_SUBSCRIBED = 25,
    NUM_ERRORS
} stratum_server_response_to_client_request_error_code;

typedef struct {
    char *id;
    char *method;
    cJSON *params;
} stratum_server_request_message;

typedef struct {
    char *id;
    char *result; // can by anything I guess
    cJSON *error;
} stratum_server_response_message;

typedef struct {
    char *error_code; // number or string
    char *message;
    char *data; // optional and specified as 'object'
} parsed_stratum_server_response_message_error;

typedef struct {
    char *job_id;                // The job ID for the job being sent in this message.
    uint8_t prev_block_hash[32]; // The hex-encoded previous block hash.
    char *coinbase_prefix;       // The hex-encoded prefix of the coinbase transaction (to precede extra nonce 2).
    char *coinbase_suffix;       // The hex-encoded suffix of the coinbase transaction (to follow extra nonce 2).
    uint8_t n_merkle_branches;   // Number of merkle branches
    uint8_t *merkle_branches;    // A JSON array containing the hex-encoded hashes needed to compute the merkle root. See Merkle Tree Hash Array.
    uint32_t version;            // The hex-encoded block version.
    uint32_t nbits;              // The hex-encoded network difficulty required for the block.
    uint32_t time;               // The hex-encoded current time for the block.
} mining_notify;

typedef struct {
    uint32_t difficulty; // The new difficulty threshold for share reporting. Shares are reported using the mining.submit message.
} mining_set_difficulty;

typedef struct {
    uint32_t version_mask; // not sure what this is yet
} mining_set_version_mask;

void check_queue_items();
void process_message(const char *message);
void parse_message(const char *message_json);
void process_mining_notify();
void process_mining_set_difficulty();
void process_mining_set_version_mask();
void parse_request(cJSON *method_json);
void parse_response(cJSON *result_json);

#endif // STRATUM_MESSAGE_COMPONENT_H
