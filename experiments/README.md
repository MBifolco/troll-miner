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
* It has 4 transactions - so merkle tree root calculation wouldn't be too crazy
* The network difficulty in 2010 was more "simpler" at 14k vs today's (Dec 2024) 100T+

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
|------------|------------|------|
| version | `version` | 4 | int32 |
| previous block hash | `previousblockhash` | 32 | char[32] |
| merkle tree root | `merkle_root` | 32 | char[32] |
| time | `timestamp` | 4 | uint32 |
| nbits | `bits` | 4 | uint32 |
| nonce | `nonce` | 4 | uint32 |

If you start searching for some of the fields in the JSON resposne, you won't find them in the block header. In some instances, like the `nonce`
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

# References

Python method to do byte-reversal
```python
def r(a):
    return "".join(reversed([a[i:i+2] for i in range(0, len(a), 2)]))
```