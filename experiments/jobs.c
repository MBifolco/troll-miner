/**
 * How to compile and execute:
 *
 * gcc jobs.c -lssl -lcrypto && ./a.out
 */
#include "utils.h"
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// BIG TODO: uint8_t vs unsigned char type ???
struct mining_subscribe_response {
    char *extranonce1;
    uint8_t extranonce2_size;
};

struct mining_notify_message {
    char *job_id;              // params[0]
    char *previous_block_hash; // params[1]
    char *coinbase_prefix;     // params[2]
    char *coinbase_suffix;     // params[3]
    char **merkle_branches;    // params[4]
    uint8_t n_merkle_branches; // no param
    char *version;             // params[5]
    char *nbits;               // params[6]
    char *time;                // params[7]
};

struct job {
    uint8_t n_merkle_branches;
    unsigned char coinbase_tx_id[32]; // double_sha256 of coinbase tx built
    unsigned char *merkle_tree_root;  // merkle tree root is 32 bytes
    unsigned char **merkle_branches;  // n branches, each will be 32 bytes
    unsigned char *previous_block_hash;
    unsigned char *version;
    unsigned char *nbits;
    unsigned char *time;
    unsigned char *nonce;
};

void reverse_bytes(unsigned char *b, size_t len) {
    unsigned char *t = malloc(len);
    for (uint8_t i = 0; i < len; ++i) {
        t[i] = b[len - i - 1];
    }
    memcpy(b, t, len);
    free(t);
}

unsigned char *coinbase_section_to_bytes(char *coinbase_str, size_t len) {
    uint8_t cb_hex_len = len / 2;
    unsigned char *cb_hex = malloc(sizeof(unsigned char) * cb_hex_len); // TODO: check if malloc res == NULL
    hex2bin(cb_hex, coinbase_str, cb_hex_len);                          // TODO: check this was successful
    return cb_hex;
}

unsigned char *build_coinbase_tx_id(char *cb_prefix, char *extranonce1, unsigned char *extranonce2, size_t extranonce2_len, char *cb_suffix) {
    unsigned char *coinbase_prefix = coinbase_section_to_bytes(cb_prefix, strlen(cb_prefix)); // TODO: check if == NULL
    unsigned char *coinbase_suffix = coinbase_section_to_bytes(cb_suffix, strlen(cb_suffix)); // TODO: check if == NULL
    unsigned char *extranonce1_hex = malloc(strlen(extranonce1) / 2);                         // TODO: check if == NULL
    hex2bin(extranonce1_hex, extranonce1, strlen(extranonce1) / 2);                           // TODO: Check result

    uint8_t extranonce1_offset = strlen(cb_prefix) / 2;
    uint8_t extranonce2_offset = extranonce1_offset + (strlen(extranonce1) / 2);
    uint8_t cb_suffix_offset = extranonce2_offset + extranonce2_len;
    uint8_t cb_tx_len = cb_suffix_offset + strlen(cb_suffix) / 2;

    unsigned char *cb_tx = malloc(cb_tx_len);
    memcpy(cb_tx, coinbase_prefix, strlen(cb_prefix) / 2);
    memcpy(cb_tx + extranonce1_offset, extranonce1_hex, strlen(extranonce1) / 2);
    memcpy(cb_tx + extranonce2_offset, extranonce2, extranonce2_len);
    memcpy(cb_tx + cb_suffix_offset, coinbase_suffix, strlen(cb_suffix) / 2);
    //   printf("Coinbase tx: ");
    //   print_hex(cb_tx, cb_tx_len);

    unsigned char *cb_tx_id = malloc(sizeof(unsigned char) * 32);
    double_sha256(cb_tx, cb_tx_len, cb_tx_id);

    free(coinbase_prefix);
    free(coinbase_suffix);
    free(extranonce1_hex);
    return cb_tx_id;
}

unsigned char *build_merkle_root(unsigned char *cb_tx_id, unsigned char **merkle_branches, size_t n_merkle_branches) {
    unsigned char *merkle_root = malloc(32);
    memcpy(merkle_root, cb_tx_id, 32);

    unsigned char temp[64];
    memcpy(temp, merkle_root, 32);

    for (uint8_t i = 0; i < n_merkle_branches; ++i) {
        memcpy(temp + 32, merkle_branches[i], 32);
        double_sha256(temp, 64, merkle_root);
        memcpy(temp, merkle_root, 32);
    }

    return merkle_root;
}

int main() {
    debug_msg("building mining_subscribe_response");
    struct mining_subscribe_response sub;
    sub.extranonce1 = "02\0";
    sub.extranonce2_size = 2;

    debug_msg("building mining_notify_message");
    struct mining_notify_message notify;
    notify.job_id = "674320f700005d59\0";
    notify.previous_block_hash = "50120119172a610421a6c3011dd330d9df07b63616c2cc1f1cd0020000000000\0";
    notify.coinbase_prefix = "01000000010000000000000000000000000000000000000000000000000000000000000000ffffffff08044c86041b\0";
    notify.coinbase_suffix = "ffffffff0100f2052a010000004341041b0e8c2567c12536aa13357b79a073dc4444acb83c4ec7a0e2f99dd7457"
                             "516c5817242da796924ca4e99947d087fedf9ce467cb9f7c6287078f801df276fdf84ac00000000\0";
    notify.version = "01000000\0";
    notify.nbits = "4c86041b\0";
    notify.time = "37221b4d\0";

    debug_msg("building mining_notify_message merkle_branches");
    // START: MERKLE BRANCH INPUT PREP
    // Overkill for this example, but it's what will inevitably need to happen
    notify.n_merkle_branches = 2;
    notify.merkle_branches = malloc(sizeof(char *) * notify.n_merkle_branches); // TODO: check if malloc res == NULL
    notify.merkle_branches[0] = "c40297f730dd7b5a99567eb8d27b78758f607507c52292d02d4031895b52f2ff\0";
    notify.merkle_branches[1] = "49aef42d78e3e9999c9e6ec9e1dddd6cb880bf3b076a03be1318ca789089308e\0";

    /**
     * 1. Transform data from struct mining_notify_message "strings" to struct job "bytes"
     * 2. Build extranonce2
     * 3. Build coinbase tx ID: double_sha256 of (coinbase1 || extranonce1 || extranonce2 || coinbase2)
     * 4. Build merkle tree root with computed coinbase tx ID & notify.merkle_branches
     * 5. Verify merkle tree root
     * 6. TBD
     */

    debug_msg("building job *j merkle_branches");
    struct job *j = malloc(sizeof(struct job));                                      // TODO: check if malloc res == NULL
    j->merkle_branches = malloc(sizeof(unsigned char *) * notify.n_merkle_branches); // TODO: check if malloc res == NULL
    for (uint8_t i = 0; i < notify.n_merkle_branches; i++) {
        j->merkle_branches[i] = malloc(sizeof(unsigned char) * 32); // TODO: check if malloc res == NULL
        hex2bin(j->merkle_branches[i], notify.merkle_branches[i], 32);
    }

    unsigned char extranonce2[2] = {0x06, 0x02};
    unsigned char *coinbase_tx_id = build_coinbase_tx_id(notify.coinbase_prefix, sub.extranonce1, extranonce2, sub.extranonce2_size,
                                                         notify.coinbase_suffix); // TODO: check if coinbase_tx_id == NULL
    printf("Coinbase tx ID: ");
    print_hex(coinbase_tx_id, 32);

    // TODO: check all the mallocs
    j->version = malloc(4);
    j->previous_block_hash = malloc(32);
    j->time = malloc(4);
    j->nbits = malloc(4);
    j->nonce = malloc(4);

    hex2bin(j->version, notify.version, strlen(notify.version) / 2);
    hex2bin(j->previous_block_hash, notify.previous_block_hash, strlen(notify.previous_block_hash) / 2);
    j->merkle_tree_root = build_merkle_root(coinbase_tx_id, j->merkle_branches, notify.n_merkle_branches); // TODO: check if successful
    hex2bin(j->time, notify.time, strlen(notify.time) / 2);
    hex2bin(j->nbits, notify.nbits, strlen(notify.time) / 2);

    printf("Merkle root: ");
    print_hex(j->merkle_tree_root, 32);

    // would usually start @ 0
    j->nonce[0] = 0x0f;
    j->nonce[1] = 0x2b;
    j->nonce[2] = 0x57;
    j->nonce[3] = 0x10;

    /**
     * At this point, the "job" would be ready. Now let's validate by building the block header,
     * double hashing it and making sure it matches block 100,000's hash
     */
    unsigned char block_header[80]; // block header is 80 bytes
    uint8_t previous_block_hash_offset = 4;
    uint8_t merkle_tree_root_offset = previous_block_hash_offset + 32;
    uint8_t time_offset = merkle_tree_root_offset + 32;
    uint8_t nbits_offset = time_offset + 4;
    uint8_t nonce_offset = nbits_offset + 4;

    memcpy(block_header, j->version, 4);
    memcpy(block_header + previous_block_hash_offset, j->previous_block_hash, 32);
    memcpy(block_header + merkle_tree_root_offset, j->merkle_tree_root, 32); // merkle tree root == 32
    memcpy(block_header + time_offset, j->time, 4);
    memcpy(block_header + nbits_offset, j->nbits, 4);
    memcpy(block_header + nonce_offset, j->nonce, 4);

    printf("Block header: ");
    print_hex(block_header, 80);

    unsigned char block_hash[32];
    double_sha256(block_header, 80, block_hash);
    reverse_bytes(block_hash, 32);
    printf("Block hash: ");
    print_hex(block_hash, 32);

    debug_msg("freeing");
    free(j->nonce);
    free(j);
    free(notify.merkle_branches);
    free(coinbase_tx_id);
    return 0;
}