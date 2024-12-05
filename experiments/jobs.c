/**
 * Example job construction based on mining.notify messaged sourced from Kano pool and BTC block 100,000
 *
 * Block 100,000:
 * {
        'id': '000000000003ba27aa200b1cecaad478d2b00432346c3f1f3986da1afd33e506',
        'height': 100000,
        'version': 1,
        'timestamp': 1293623863,
        'tx_count': 4,
        'size': 957,
        'weight': 3828,
        'merkle_root': 'f3e94742aca4b5ef85488dc37c06c3282295ffec960994b2c0d5ac2a25a95766',
        'previousblockhash': '000000000002d01c1fccc21636b607dfd930d31d01c3a62104612a1719011250',
        'mediantime': 1293622620,
        'nonce': 274148111,
        'bits': 453281356,
        'difficulty': 14484.162361225399
   }
 *

    Field           Received Format     Usage in Block Header
    -----           ---------------     ---------------------
    prevhash        Big-endian          Little-endian
    coinbase1       Hex string          Used as-is
    coinbase2       Hex string          Used as-is
    merkle_branch   Big-endian          Big-endian (for Merkle root calculation)
    version         Big-endian          Little-endian
    nbits           Big-endian          Big-endian
    ntime           Big-endian          Little-endian

 * Sample job message, for block 100,000
 * {
   "id" : null,
   "method" : "mining.notify",
   "params" : [
      // params[0] - job id
      "674320f700005d59",

      // params[1] - previous block hash (big-endian)
      "000000000002d01c1fccc21636b607dfd930d31d01c3a62104612a1719011250",

      // NOTE: params [2] & [3] were decomposed from the coinbase tx of block 100,000
      https://mempool.space/api/tx/8c14f0db3df150123e6f3dbbf30f8b955a8249b62ac1d1ff16284aefa3d06d87/hex

      // params [2] - prefix of the coinbase transaction (to precede extra nonce 2).
      "01000000010000000000000000000000000000000000000000000000000000000000000000ffffffff08044c8604",

      // NOTE: extra nonce 2 for coinbase tx of block 100,000 = 1b020602 (like coinbase1 & coinbase2, endianess does not apply)

      // params [3] - suffix of the coinbase transaction (to follow extra nonce 2).
      "ffffffff0100f2052a010000004341041b0e8c2567c12536aa13357b79a073dc4444acb83c4ec7a0e2f99dd7457516c5817242da796924ca4e99947d087fedf9ce467cb9f7c6287078f801df276fdf84ac00000000",

      // NOTE: How did I come up with these merkle branches when block 100,000 had 4 transactions?
      // 1) The first transaction we'll compute from double_sha of coinbase1_extranonce2_coinbase2
      // 2) Generally mining.notify sends branches that would require the smallest number of computations. So given txs [TX1 (coinbase), TX2, TX3, TX4] yields
          Root
         /    \
       H12    H34
       / \    / \
      H1 H2  H3 H4

      We only need [H2, H34] to reach the root. So the second element is the double_sha of tx3 & tx4 concatenated.
      NOTE: H2 is actually reversed here (because it's usually little-endian). See: https://medium.com/@stolman.b/1-4-hashing-transactions-txids-to-find-the-merkle-root-3aa5255f8de8
      // params[4] - array of merkle branches (big-endian).
      [
         "c40297f730dd7b5a99567eb8d27b78758f607507c52292d02d4031895b52f2ff",
         "49aef42d78e3e9999c9e6ec9e1dddd6cb880bf3b076a03be1318ca789089308e"
      ],

      // params[5] - version (big-endian)
      "00000001",

      // params[6] - difficulty ("bits" field of block header; big-endian; this is an encoding: https://developer.bitcoin.org/reference/block_chain.html#target-nbits)
      "4c86041b",

      // params[7] - current time for the block (big-endian - hex(1293623863)[2:])
      "4d1b2237",
      true
   ]
}
 */

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "jobs.h"

/**
 * https://reference.cash/mining/stratum-protocol
 * extranonce2 size is provided in stratum response to mining.subscribe notify
 * {"result":[[["mining.notify","<SESSION_ID>"]],"<EXTRANONCE_1>",<SIZEOF_EXTRANONCE_2>],"id":2,"error":null}
 * Example: {"result":[[["mining.notify","67f68537"]],"61dd9767",8],"id":2,"error":null}
 * "61dd9767" is the value of extranonce1 and extranonce2 has a size of 8 bytes
 *
 * I'm starting to see why kano has a `pool` object...holds data from startum responses that change somewhat infrequently
 */
struct mining_subscribe_response
{
    char *extranonce1;
    uint8_t extranonce2_size;
};

struct mining_notify_message
{
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

struct job
{
    int merkle_branches_n;
    unsigned char merkle_tree_root[32]; // merkle tree root is 32 bytes
    unsigned char **merkle_branches;    // n branches, each will be 32 bytes
};

void debug_msg(char *msg, bool enabled)
{
    if (enabled)
    {
        printf("DEBUG: %s\n", msg);
    }
}

void print_hex(unsigned char *h, size_t len)
{
    for (uint8_t i = 0; i < len; ++i)
    {
        printf("%02x", h[i]);
    }
    printf("\n");
}

int main()
{
    /**
     * https://mempool.space/api/tx/8c14f0db3df150123e6f3dbbf30f8b955a8249b62ac1d1ff16284aefa3d06d87/hex
     * Because block 100,000 is quite old, decomposing the serialized coinbase tx, we see that
     * extranonce1 == 0x02   and is only 1 byte
     * extranonce2 == 0x0602 and is only 2 bytes
     *
     * extranonce1 is generally 4 to 8 bytes
     */
    bool debugging = false;

    debug_msg("DEBUG: building mining_subscribe_response", debugging);
    struct mining_subscribe_response sub;
    sub.extranonce1 = "02\0";
    sub.extranonce2_size = 2;

    debug_msg("DEBUG: building mining_notify_message", debugging);
    struct mining_notify_message notify;
    notify.job_id = "674320f700005d59\0";
    notify.previous_block_hash = "000000000002d01c1fccc21636b607dfd930d31d01c3a62104612a1719011250\0";
    notify.coinbase_prefix = "01000000010000000000000000000000000000000000000000000000000000000000000000ffffffff08044c8604\0";
    notify.coinbase_suffix = "ffffffff0100f2052a010000004341041b0e8c2567c12536aa13357b79a073dc4444acb83c4ec7a0e2f99dd7457516c5817242da796924ca4e99947d087fedf9ce467cb9f7c6287078f801df276fdf84ac00000000\0";
    notify.version = "00000001\0";
    notify.nbits = "4c86041b\0";
    notify.time = "4d1b2237\0";

    debug_msg("DEBUG: building mining_notify_message merkle_branches", debugging);
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

    debug_msg("DEBUG: building job *j merkle_branches", debugging);
    struct job *j = malloc(sizeof(struct job));                                      // TODO: check if malloc res == NULL
    j->merkle_branches = malloc(sizeof(unsigned char *) * notify.n_merkle_branches); // TODO: check if malloc res == NULL
    for (uint8_t i = 0; i < notify.n_merkle_branches; i++)
    {
        debug_msg("DEBUG: allocating merkle_branch", debugging);
        j->merkle_branches[i] = malloc(sizeof(unsigned char) * 32); // TODO: check if malloc res == NULL
        debug_msg("DEBUG: hex2bin", debugging);
        hex2bin(j->merkle_branches[i], notify.merkle_branches[i], 32);

        printf("Merkle branch %d built: ", i);
        print_hex(j->merkle_branches[i], 32);
    }

    debug_msg("DEBUG: free j", debugging);
    free(j);
    debug_msg("DEBUG: free merkle_branches", debugging);
    free(notify.merkle_branches); // TODO: Do we need to free each "column"? Probably...
    return 0;
}