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
 * Sample job message, for block 100,000
 * {
   "id" : null,
   "method" : "mining.notify",
   "params" : [
      // params[0] - job id
      "674320f700005d59",

      // params[1] - previous block hash (little-endian)
      "50120119172a610421a6c3011dd330d9df07b63616c2cc1f1cd0020000000000",

      // TODO: reverse engineer this from block 100,000
      // params [2] - prefix of the coinbase transaction (to precede extra nonce 2).
      "01000000010000000000000000000000000000000000000000000000000000000000000000ffffffff3603ee510d00042beb4d6704572cd7040c",

      // TODO: reverse engineer this from block 100,000
      // params [3] - suffix of the coinbase transaction (to follow extra nonce 2).
      "0a2ef09f90882e12204b616e6f506f6f6c20283d4f2e4f3d2920ffffffff0260660113000000001976a9148375f59e2771fe785cc069654fed5242223ecd5088ac0000000000000000266a24aa21a9ed32b3c03125272915a4c69c04b57f6b0afe11242545d5293c3a3c75b2782c101d00000000",

      // params[4] - array of merkle branches (these are the actual block 100,000 tx IDs, but reversed to little-endian, just like the merkle branches would be).
      [
         '876dd0a3ef4a2816ffd1c12ab649825a958b0ff3bb3d6f3e1250f13ddbf0148c',
         'c40297f730dd7b5a99567eb8d27b78758f607507c52292d02d4031895b52f2ff',
         'c46e239ab7d28e2c019b6d66ad8fae98a56ef1f21aeecb94d1b1718186f05963',
         '1d0cb83721529a062d9675b98d6e5c587e4a770fc84ed00abc5a5de04568a6e9'
      ],

      // params[5] - version (little-endian)
      "10000000",

      // params[6] - difficulty ("bits" field of block header - this is an encoding: https://developer.bitcoin.org/reference/block_chain.html#target-nbits)
      "1b04864c",

      // params[7] - current time for the block (little-endian - byte reversed result of timestamp: byte_flip(hex(1293623863)[2:]))
      "37221b4d",
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

/**
 * Pulled from https://github.com/kanoi/cgminer/blob/e8d49a56163419fedca385e61abf88d55d8cc30d/util.c#L880C1-L929C2
 */
bool hex2bin(unsigned char *p, const char *hexstr, size_t len)
{
    int nibble1, nibble2;
    unsigned char idx;
    bool ret = false;

    while (*hexstr && len)
    {
        if (unlikely(!hexstr[1]))
        {
            return ret;
        }

        idx = *hexstr++;
        nibble1 = hex2bin_tbl[idx];
        idx = *hexstr++;
        nibble2 = hex2bin_tbl[idx];

        if (unlikely((nibble1 < 0) || (nibble2 < 0)))
        {
            return ret;
        }

        *p++ = (((unsigned char)nibble1) << 4) | ((unsigned char)nibble2);
        --len;
    }

    if (likely(len == 0 && *hexstr == 0))
        ret = true;
    return ret;
}

struct job
{
    int merkle_branches_n;
    unsigned char merkle_tree_root[32]; // merkle tree root is 32 bytes
    // unsigned char **merkle_branches; // n branches, each 32 bytes
};

int main()
{
    // 64 characters is 32 bytes
    char input_merkle_branches[4][64] = {
        "8c14f0db3df150123e6f3dbbf30f8b955a8249b62ac1d1ff16284aefa3d06d87",
        "fff2525b8931402dd09222c50775608f75787bd2b87e56995a7bdd30f79702c4",
        "6359f0868171b1d194cbee1af2f16ea598ae8fad666d9b012c8ed2b79a236ec4",
        "e9a66845e05d5abc0ad04ec80f774a7e585c6e8db975962d069a522137b80c1d"};

    unsigned char *p = malloc(sizeof(unsigned char) * 64);
    if (p == NULL)
    {
        printf("unsigned char *p malloc failed: %d", errno);
        return -1;
    }

    struct job *j = malloc(sizeof(struct job));
    if (j == NULL)
    {
        printf("struct job *j malloc failed: %d", errno);
        free(p);
        return -1;
    }

    hex2bin(p, input_merkle_branches[0], 64);

    for (size_t i = 0; i < 64; i++)
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