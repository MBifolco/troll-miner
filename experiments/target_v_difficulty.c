#include <errno.h>
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
static const double bits160 = 1461501637330902918203684832716283019655932542976.0;
static const double bits128 = 340282366920938463463374607431768211456.0;
static const double bits96 = 79228162514264337593543950336.0;
static const double bits64 = 18446744073709551616.0;
static const double bits32 = 4294967296.0;

void calc_target32(uint8_t *target, double diff) {
    uint32_t *data32, h32;
    double d32, dcut32;
    /**
     * We have to start from bits192, because there truediffone is 0x00000000FFFF....
     * meaning there's already an offset - so to do the 32bit equivalent operations of
     * target calc, we start at bits192, offset 24 in target, to account for the first
     * 4 bytes of truediffone being 0. After that, we can proceed with 32 bit segments.
     */
    double segments[6] = {bits192, bits160, bits128, bits96, bits64, bits32};
    uint8_t segment_offsets_in_target[6] = {24, 20, 16, 12, 8, 4};

    d32 = truediffone;
    d32 /= diff;

    for (int i = 0; i < 6; i++) {
        dcut32 = d32 / segments[i];
        h32 = dcut32;
        data32 = (uint32_t *)(target + segment_offsets_in_target[i]);
        *data32 = htole32(h32);
        dcut32 = h32;
        dcut32 *= segments[i];
        d32 -= dcut32;
    }

    h32 = d32;
    data32 = (uint32_t *)(target);
    *data32 = htole32(h32);
}

void calc_target64(uint8_t *target, double diff) {
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
     */

    if (argc != 2) {
        printf("Please provide a difficulty value you'd like transformed into a target and mask.\n");
    }

    char *p;
    double diff = strtod(argv[1], &p);
    if (diff == 0) {
        /* If the value provided was out of range, display a warning message */
        if (errno == ERANGE)
            printf("The value provided was out of range\n");
    }

    uint8_t target[32];

    calc_target32(target, diff);
    printf("Target (32bit):\t");
    for (int i = 31; i >= 0; i--) {
        printf("%02x", target[i]);
    }
    printf("\n");

    calc_target64(target, diff);
    printf("Target (64bit):\t");
    for (int i = 31; i >= 0; i--) {
        printf("%02x", target[i]);
    }
    printf("\n");

    uint8_t mask[32];
    calc_mask(mask, target);
    printf("Mask:\t\t");
    for (int i = 31; i >= 0; i--) {
        printf("%02x", mask[i]);
    }
    printf("\n");
}