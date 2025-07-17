#ifndef BM1366_H_
#define BM1366_H_

#include "common.h"
#include "driver/gpio.h"
#include "job.h"
#include <stdint.h>
// #include "mining.h"

#define FREQUENCY_MS 2000
#define ASIC_DIFFICULTY 256

#define SERIALTX_DEBUG false
#define SERIALRX_DEBUG false
#define DEBUG_WORK false // causes insane amount of debug output
#define DEBUG_JOBS false // causes insane amount of debug output

// TODO: Maybe we build work this way instead...?
// typedef struct __attribute__((__packed__)) {
//     uint8_t work_id;
//     uint8_t num_midstates;
//     uint8_t starting_nonce[4];
//     uint8_t nbits[4];
//     uint8_t ntime[4];
//     uint8_t merkle_root[32];
//     uint8_t prev_block_hash[32];
//     uint8_t version[4];
// } work;

typedef enum {
    JOB_PACKET = 0,
    CMD_PACKET = 1,
} packet_type_t;

void asic_init();
void set_version_mask(uint32_t version_mask);
void send_work(uint8_t block_header[BLOCK_HEADER_SIZE]);
void send_job_difficulty(int difficulty);
int count_asic_chips(uint16_t asic_count, uint16_t chip_id, int chip_id_response_length);
#endif /* BM1366_H_ */