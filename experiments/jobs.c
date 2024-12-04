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

      // params[1] - previous block hash (little-endian) TODO: THIS COMES IN REVERSED - so... don't forget to reverse this!
      "000000000002d01c1fccc21636b607dfd930d31d01c3a62104612a1719011250",

      // params [2] - prefix of the coinbase transaction (to precede extra nonce 2).
      "01000000010000000000000000000000000000000000000000000000000000000000000000ffffffff3603ee510d00042beb4d6704572cd7040c",

      // params [3] - suffix of the coinbase transaction (to follow extra nonce 2).
      "0a2ef09f90882e12204b616e6f506f6f6c20283d4f2e4f3d2920ffffffff0260660113000000001976a9148375f59e2771fe785cc069654fed5242223ecd5088ac0000000000000000266a24aa21a9ed32b3c03125272915a4c69c04b57f6b0afe11242545d5293c3a3c75b2782c101d00000000",

      // params[4] - array of tx IDs - needed to build merkle tree root - (all are little-endian)
      [
         '8c14f0db3df150123e6f3dbbf30f8b955a8249b62ac1d1ff16284aefa3d06d87',
         'fff2525b8931402dd09222c50775608f75787bd2b87e56995a7bdd30f79702c4',
         '6359f0868171b1d194cbee1af2f16ea598ae8fad666d9b012c8ed2b79a236ec4',
         'e9a66845e05d5abc0ad04ec80f774a7e585c6e8db975962d069a522137b80c1d'
      ],

      // params[5] - version
      "10000000",

      // params[6] - difficulty ("bits" field of block header)
      "1b04864c",

      // params[7] - current time for the block (hex() of block's timestamp field)
      "4d1b2237",
      true
   ]
}
 */

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

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

    struct job *j = malloc(sizeof(struct job));
    if (j == NULL)
    {
        printf("struct job memory allocation failed: %d", errno);
        return -1;
    }

    j->merkle_branches_n = 4;
    //
    // build_merkle_tree_root()

    free(j);
    return 0;
}