#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * SOURCE: https://github.com/kanoi/cgminer/blob/master/cgminer.c#L7843 - set_target
 *
 * truediffone == 0x00000000FFFF0000000000000000000000000000000000000000000000000000
 * Generate a 256 bit binary LE target by cutting up diff into 64 bit sized
 * portions or vice versa.
 *
 * calc_target === cgminer set_target
 */
static const double truediffone = 26959535291011309493156476344723991336010898738574164086137773096960.0;
static const double bits192 = 6277101735386680763835789423207666416102355444464034512896.0;
static const double bits128 = 340282366920938463463374607431768211456.0;
static const double bits64 = 18446744073709551616.0;

void calc_target(uint8_t *target, double diff) {
    uint64_t *data64, h64;
    double d64, dcut64;

    d64 = truediffone;
    d64 /= diff;

    dcut64 = d64 / bits192;
    h64 = dcut64;

    data64 = (uint64_t *)(target + 24);
    *data64 = htole64(h64);
    dcut64 = h64;
    dcut64 *= bits192;
    d64 -= dcut64;

    dcut64 = d64 / bits128;
    h64 = dcut64;
    data64 = (uint64_t *)(target + 16);
    *data64 = htole64(h64);
    dcut64 = h64;
    dcut64 *= bits128;
    d64 -= dcut64;

    dcut64 = d64 / bits64;
    h64 = dcut64;
    data64 = (uint64_t *)(target + 8);
    *data64 = htole64(h64);
    dcut64 = h64;
    dcut64 *= bits64;
    d64 -= dcut64;

    h64 = d64;
    data64 = (uint64_t *)(target);
    *data64 = htole64(h64);
}

void calc_mask(uint8_t *mask, uint8_t *target) {
    for (int i = 0; i < 32; i++) {
        if (target[i] > 0) {
            mask[i] = 0xff;
        } else {
            mask[i] = 0;
        }
    }
}

int main(int argc, char *argv[]) {
    /**
     * TODO CLI args:
     * - direction (i.e. diff2target vs target2diff)
     * - support huge numbers (i.e. uint32_t / unsigned long)
     */

    if (argc != 2) {
        printf("Please provide a difficulty value you'd like transformed into a target and mask.\n");
    }

    // TODO: #s > MAX_INT will produce undefined behavior. Should use something like strtol for more robust support
    int diff = atoi(argv[1]);
    uint8_t target[32];
    calc_target(target, (double)diff);
    printf("Target:\t");
    for (int i = 31; i >= 0; i--) {
        printf("%02x", target[i]);
    }
    printf("\n");

    uint8_t mask[32];
    calc_mask(mask, target);
    printf("Mask:\t");
    for (int i = 31; i >= 0; i--) {
        printf("%02x", mask[i]);
    }
    printf("\n");
}