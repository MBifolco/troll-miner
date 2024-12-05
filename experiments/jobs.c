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

    Field	        Received Format	    Usage in Block Header
    -----           ---------------     ---------------------
    prevhash	    Big-endian	        Little-endian
    coinbase1	    Hex string	        Used as-is
    coinbase2	    Hex string	        Used as-is
    merkle_branch	Big-endian	        Big-endian (for Merkle root calculation)
    version	        Big-endian	        Little-endian
    nbits	        Big-endian	        Big-endian
    ntime	        Big-endian	        Little-endian

 * Sample job message, for block 100,000
 * {
   "id" : null,
   "method" : "mining.notify",
   "params" : [
      // params[0] - job id
      "674320f700005d59",

      // params[1] - previous block hash (big-endian)
      "000000000002d01c1fccc21636b607dfd930d31d01c3a62104612a1719011250",

      // params [2] & [3] were decomposed from the coinbase tx of block 100,000 - https://mempool.space/api/tx/8c14f0db3df150123e6f3dbbf30f8b955a8249b62ac1d1ff16284aefa3d06d87/hex
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
#include "jobs.h"

struct job
{
    int merkle_branches_n;
    unsigned char merkle_tree_root[32]; // merkle tree root is 32 bytes
    // unsigned char **merkle_branches; // n branches, each 32 bytes
};

int main()
{
    // 64 character hex strings ===> 32 bytes
    char input_merkle_branches[4][64] = {
        "c40297f730dd7b5a99567eb8d27b78758f607507c52292d02d4031895b52f2ff",
        "49aef42d78e3e9999c9e6ec9e1dddd6cb880bf3b076a03be1318ca789089308e"};

    unsigned char *p = malloc(sizeof(unsigned char) * 32);
    if (p == NULL)
    {
        printf("unsigned char *p malloc failed: %d\n", errno);
        return -1;
    }

    struct job *j = malloc(sizeof(struct job));
    if (j == NULL)
    {
        printf("struct job *j malloc failed: %d\n", errno);
        free(p);
        return -1;
    }

    bool r = hex2bin(p, input_merkle_branches[0], 32);
    if (!r)
    {
        printf("Failed to convert %s to byte representation\n");
        return -1;
    }

    for (size_t i = 0; i < 32; i++)
    {
        printf("Byte %zu: 0x%02X\n", i, p[i]);
    }

    // j->merkle_branches_n = 4;
    // how do we deal with the merkle branches?
    // build_merkle_tree_root()

    free(j);
    free(p);
    return 0;
}