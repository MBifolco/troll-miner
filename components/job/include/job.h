#ifndef JOB_COMPONENT_H
#define JOB_COMPONENT_H

#include "cJSON.h"
#include <stdint.h>
#include <stdbool.h>

#define MAX_MERKLE_BRANCHES 32
#define HASH_SIZE 32
#define COINBASE_SIZE 100
#define COINBASE2_SIZE 128

// docs on reqeusts and responses, methods and results 
// https://reference.cash/mining/stratum-protocol
typedef enum
{
    UNKNOWN,
    MINING_NOTIFY,
    MINING_SET_DIFFICULTY,
    MINING_SET_VERSION_MASK,
    CLIENT_RECONNECT
} stratum_server_request_method;

typedef enum
{
    UNKNOWN,
    STRATUM_RESULT,
    STRATUM_RESULT_SETUP,
    STRATUM_RESULT_VERSION_MASK,
    STRATUM_RESULT_SUBSCRIBE,
} stratum_server_response_result;

static const int  STRATUM_ID_CONFIGURE    = 1;
static const int  STRATUM_ID_SUBSCRIBE    = 2;

typedef struct
{
    char *job_id;
    char *prev_block_hash;
    char *coinbase_1;
    char *coinbase_2;
    uint8_t *merkle_branches;
    size_t n_merkle_branches;
    uint32_t version;
    uint32_t version_mask;
    uint32_t target;
    uint32_t ntime;
    uint32_t difficulty;
} mining_notify;

typedef struct
{
    char * extranonce_str;
    int extranonce_2_len;
    int64_t message_id;
    stratum_method method;
    int should_abandon_work;
    mining_notify *mining_notification;
    uint32_t new_difficulty;
    uint32_t version_mask;
    bool response_success;
} parsed_message;

void receive_message(const char *message_json);
void parse_message(parsed_message * message, const char * message_json);

#endif // JOB_COMPONENT_H
