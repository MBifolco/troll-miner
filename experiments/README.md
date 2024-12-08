# Experiments - Job Construction

Some high level definitions, as we currently understand them:
* A `job` is the "block template" data a stratum server sends to a miner via `mining.notify` messages
* `work` is a fully formed block header which the miner sends to the ASIC

The purpose of this experiment is to
1. Find out what stratum messages are involved when creating a "job"
2. Learn how to build a `job` from the relevant stratum messages
3. Verify that a given `job` could produce `work` that an ASIC would work on

# Compilation & Execution

```
$ gcc jobs.c -lssl -lcrypto && ./a.out
```

# Experiment Details

At the time of this writing, we've only seen a handful of starter stratum messages from [KanoPool](https://kano.is) and have
only just begun to truly understand what is involved in the syntax and symantics of those fields and the block header fields.

Instead, why not start from a mined block and work backwards - reverse engineer the stratum messages that likely could have
been sent at the time for it. Our block candidate was [block 100,000](https://mempool.space/block/000000000003ba27aa200b1cecaad478d2b00432346c3f1f3986da1afd33e506). We felt this was a good candidate because
* It has 4 transactions - merkle tree root calculation wouldn't be too crazy
* The network difficulty in 2010 was "simpler" at 14k vs today's (Dec 2024) 100T+

## Block 100,000

[Here's a "friendly" version of the block details](https://mempool.space/api/block/000000000003ba27aa200b1cecaad478d2b00432346c3f1f3986da1afd33e506)

```json
{
    "id": "000000000003ba27aa200b1cecaad478d2b00432346c3f1f3986da1afd33e506",
    "height": 100000,
    "version": 1,
    "timestamp": 1293623863,
    "tx_count": 4,
    "size": 957,
    "weight": 3828,
    "merkle_root": "f3e94742aca4b5ef85488dc37c06c3282295ffec960994b2c0d5ac2a25a95766",
    "previousblockhash": "000000000002d01c1fccc21636b607dfd930d31d01c3a62104612a1719011250",
    "mediantime": 1293622620,
    "nonce": 274148111,
    "bits": 453281356,
    "difficulty": 14484.162361225399
}
```

[Here's the hex-encoded block header](https://mempool.space/api/block/000000000003ba27aa200b1cecaad478d2b00432346c3f1f3986da1afd33e506/header)
```
0100000050120119172a610421a6c3011dd330d9df07b63616c2cc1f1cd00200000000006657a9252aacd5c0b2940996ecff952228c3067cc38d4885efb5a4ac4247e9f337221b4d4c86041b0f2b5710
```

We ultimately need to be able to construct some variation of the hex-encoded block header for the ASIC.

[Here's a quick reminder of what's in a block header](https://developer.bitcoin.org/reference/block_chain.html#block-headers)
| Field Name | JSON Field Name | # of bytes | Type |
|------------|-----------------|------------|------|
| version | `version` | 4 | int32 |
| previous block hash | `previousblockhash` | 32 | char[32] |
| merkle tree root | `merkle_root` | 32 | char[32] |
| time | `timestamp` | 4 | uint32 |
| nbits | `bits` | 4 | uint32 |
| nonce | `nonce` | 4 | uint32 |

If you start searching for some of the fields in the JSON response, you won't find them in the block header. In some instances, like the `nonce`
and `bits` fields, they need to be converted to hex. In the other cases, like `merkle_root` and `previousblockhash`, it's because their byte order is
reversed in the block header. This is important because of each field's [endianess](https://www.rapidtables.com/prog/endianess.html). Not only that,
the endianess of the bytes sent in a `mining.notify` message do not necessarily match the endianess of the particular block header field.

Let's deconstruct some of the fields from the JSON blob and turn them into something we can find in the block header (NOTE: this will be useful
when we build the `mining.notify` message later!)

`merkle_root`
```
f3e94742aca4b5ef85488dc37c06c3282295ffec960994b2c0d5ac2a25a95766
------>
6657a9252aacd5c0b2940996ecff952228c3067cc38d4885efb5a4ac4247e9f3
```

`previousblockhash`
```
000000000002d01c1fccc21636b607dfd930d31d01c3a62104612a1719011250
------>
50120119172a610421a6c3011dd330d9df07b63616c2cc1f1cd0020000000000
```

If we know the end state of the block header and it's hash, we should be able to work backwards to a plausible set of stratum message's needed to construct said block header.

## Pool stratum messages

The two main messages are
1. The response from a pool when a client sends the `mining.subscribe` request
2. The `mining.notify` message sent from a pool to a client.

The `mining.notify` message contains the majority of information needed to build a header, however, the `mining.subscribe` pool response contains two key pieces of information:
1. `extranonce1`
2. `extranonce2` byte length

The structures of the `mining.notify` message and the `mining.subscribe` response can be found here: https://reference.cash/mining/stratum-protocol

### Coinbase Transaction and Merkle Tree Root

The `mining.notify` contains, most of the information we need to build the block header, but not everything. The miner needs to build the coinbase transaction and from it, build the
coinbase transaction ID (aka - double SHA256 of the coinbase transaction). Only then can the miner compute the merkle tree root based on the calculated coinbase tx ID + the merkle branches
provided by `mining.notify`. 

But how do we build the coinbase transaction? `params[2]` and `params[3]` in `mining.notify` give the "prefix" and "suffix" of the hex-encoded coinbase transaction. This is where
we see the importance of the `mining.subscribe` response.

```
coinbase tx = coinbase_prefix || extranonce1 || extranonce2 || coinbase_suffix
coinbase tx ID = double_SHA256(coinbase tx)
```

When a nonce space is exhausted for a block header, the miner modifies the value of `extranonce2`, recomputes the merkle tree root, and explores the next nonce space.

### Block 100,000 Coinbase Transaction

Per the last section, we need to deconstruct block 100,000's coinbase transaction to get the correct data in our test stratum messages.

Here's a friendly version of block 100,000's coinbase transaction: https://mempool.space/tx/8c14f0db3df150123e6f3dbbf30f8b955a8249b62ac1d1ff16284aefa3d06d87

[We need the hex encoding though](https://mempool.space/api/tx/8c14f0db3df150123e6f3dbbf30f8b955a8249b62ac1d1ff16284aefa3d06d87/hex):
```
01000000010000000000000000000000000000000000000000000000000000000000000000ffffffff08044c86041b020602ffffffff0100f2052a010000004341041b0e8c2567c12536aa13357b79a073dc4444acb83c4ec7a0e2f99dd7457516c5817242da796924ca4e99947d087fedf9ce467cb9f7c6287078f801df276fdf84ac00000000
```

With this, we can deconstruct the fields we need:
* `coinbase_prefix`: `01000000010000000000000000000000000000000000000000000000000000000000000000ffffffff08044c86041b`
* `extranonce1`: `02`
* `extranonce2`: `0602`
* `coinbase_suffix`: `ffffffff0100f2052a010000004341041b0e8c2567c12536aa13357b79a073dc4444acb83c4ec7a0e2f99dd7457516c5817242da796924ca4e99947d087fedf9ce467cb9f7c6287078f801df276fdf84ac00000000`

### Block 100,000 stratum messages

#### `mining.subscribe`

After decomposing the coinbase transaction, we can build the `minings.subscribe` response:

```json
{
    "result": [
        [
            ["mining.notify","67f68537"]
        ],
        "02", // extranonce1
        2     // extranonce2 byte length
    ],
    "id":2,
    "error":null
}
```

#### `mining.notify`

With the decomposed coinbase transaction and the information available about [block 100,000](https://mempool.space/block/000000000003ba27aa200b1cecaad478d2b00432346c3f1f3986da1afd33e506?showDetails=true&view=actual#details)
we can construct the `mining.notify` message.

OK - that's actually a lie. We'll build it first and then dig into something I've completely glossed over: merkle branches in `mining.notify`.

```json
{
   "id" : null,
   "method" : "mining.notify",
   "params" : [
      // params 0 - job id
      "674320f700005d59",

      // params 1 - previous block hash
      "000000000002d01c1fccc21636b607dfd930d31d01c3a62104612a1719011250",

      // params 2 - prefix of the coinbase transaction
      "01000000010000000000000000000000000000000000000000000000000000000000000000ffffffff08044c86041b",

      // params 3 - suffix of the coinbase transaction
      "ffffffff0100f2052a010000004341041b0e8c2567c12536aa13357b79a073dc4444acb83c4ec7a0e2f99dd7457516c5817242da796924ca4e99947d087fedf9ce467cb9f7c6287078f801df276fdf84ac00000000",

      // params 4 - array of merkle branches
      [
         "c40297f730dd7b5a99567eb8d27b78758f607507c52292d02d4031895b52f2ff",
         "49aef42d78e3e9999c9e6ec9e1dddd6cb880bf3b076a03be1318ca789089308e"
      ],

      // params 5 - version
      "00000001",

      // params 6 - difficulty (take the `bits` value above and in python: hex(453281356)[2:], then reverse the bytes - this field is big-endian
      "4c86041b",

      // params 7 - current time for the block (take the `timestamp` value above and in python: hex(1293623863)[2:])
      "4d1b2237",

      // params 8
      true
   ]
}
```

### A note about merkle tree branches...


## Building Block 100,000


# References

### Python method to do byte-reversal
```python
def r(a):
    return "".join(reversed([a[i:i+2] for i in range(0, len(a), 2)]))
```

### Field Endianess Table
|Field           |Received Format     |Usage in Block Header|
|-----           |---------------     |---------------------|
|prevhash        |Big-endian          |Little-endian|
|coinbase1       |Hex string          |Used as-is|
|coinbase2       |Hex string          |Used as-is|
|extranonce1     |Hex string          |Used as-is|
|extranonce2     |N/A                 |Used as-is|
|merkle_branch   |Big-endian          |Big-endian (Merkle tree root)|
|version         |Big-endian          |Little-endian|
|nbits           |Big-endian          |Big-endian|
|ntime           |Big-endian          |Little-endian|
|nonce           |N/A                 |Little-endian|
