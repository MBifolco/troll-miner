#ifndef FAYKSIC_H_
#define FAYKSIC_H_

#include "common.h"
#include "driver/gpio.h"
#include <stdint.h>
//#include "mining.h"

#define ASIC_FAYKSIC_JOB_FREQUENCY_MS 2000
#define FAYKSIC_ASIC_DIFFICULTY 256

#define FAYKSIC_SERIALTX_DEBUG false
#define FAYKSIC_SERIALRX_DEBUG false
#define FAYKSIC_DEBUG_WORK false //causes insane amount of debug output
#define FAYKSIC_DEBUG_JOBS false //causes insane amount of debug output


typedef struct
{
    uint32_t version;
    uint32_t version_mask;
    uint8_t prev_block_hash[32];
    uint8_t prev_block_hash_be[32];
    uint8_t merkle_root[32];
    uint8_t merkle_root_be[32];
    uint32_t ntime;
    uint32_t target; // aka difficulty, aka nbits
    uint32_t starting_nonce;

    uint8_t num_midstates;
    uint8_t midstate[32];
    uint8_t midstate1[32];
    uint8_t midstate2[32];
    uint8_t midstate3[32];
    uint32_t pool_diff;
    char *jobid;
    char *extranonce2;
} job_work;

typedef struct __attribute__((__packed__))
{
    uint8_t work_id;
    uint8_t num_midstates;
    uint8_t starting_nonce[4];
    uint8_t nbits[4];
    uint8_t ntime[4];
    uint8_t merkle_root[32];
    uint8_t prev_block_hash[32];
    uint8_t version[4];
} work;

typedef enum
{
    JOB_PACKET = 0,
    CMD_PACKET = 1,
} packet_type_t;


void send_work(job_work * job_work);

#endif /* FAYKSIC_H_ */