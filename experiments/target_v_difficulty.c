/**
 * Compile:
 * gcc target_v_difficulty.c
 *
 * Usage:
 * ./a.out 442
 * ./a.out 1
 * ./a.out 4284.532
 * ./a.out 108522647629298.2
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void ESP32_calc_target(double diff) {
    /**
     * ESP32s operate on a 32-bit architecture, so the following has been tuned for that,
     * specifically using 32-bit unsigned integer types.
     *
     * Once you start pumping numbers near 2^32 - 1 (i.e. 4,294,967,295) the result is just 0s
     * because those numbers are too big and this isn't precise enough. But if we can get an ESP32
     * to handle enough ASICs where a mining pool would return a pool difficulty in the billions,
     * I'd say that's a pretty good problem, but we can also deal with it later and optimize for
     * efficient target computation within the architecture we'll exist in.
     *
     * The other functions i.e. kano's algorithm, are better suited for huge numbers, i.e.
     * current network difficulty (Dec 21, 2024 @ 108,522,647,629,298.2).
     *
     * This is effectively the "long division" algorithm. It's also worth mentioning it's faster
     * to avoid using the % operator to compute the remainder.
     *
     * Reasoning:
     * Addition/Subtraction: Very fast (1 cycle or close to 1 cycle).
     * Multiplication: Slightly slower (often 3–6 cycles, depending on the microcontroller).
     * Division/Modulus: Significantly slower (10–40 cycles or more, depending on the size of the numbers).
     *
     * On a 32-bit microcontroller:
     * a % b for uint32_t involves a full division (a / b) and an additional multiplication and subtraction to compute the remainder.
     *
     * For uint64_t, the cost increases substantially because it requires emulating 64-bit division using multiple 32-bit operations.
     */
    if (diff > INT32_MAX) {
        printf("ESP32 Target: Cannot compute target - difficulty > %d\n", INT32_MAX);
        return;
    }
    const uint32_t n_truediffone[8] = {0, 0xFFFF0000, 0, 0, 0, 0, 0, 0};
    uint32_t current = 0;
    uint32_t result[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    // for (int i = 7; i >= 0; i--) {
    //     // Get the current chunk
    //     current = (current << 31) | n_truediffone[i];
    //     // Store the result
    //     result[i] = (uint32_t)(current / diff);
    //     // Pull out the remainder
    //     current %= (uint32_t)diff;
    // }

    // Alternative to above, but w/out % op
    for (int i = 7; i >= 0; i--) {
        // Get the current chunk
        current = (current << 31) | n_truediffone[i];

        // Store the result
        result[i] = (uint32_t)(current / diff);

        // Pull out the remainder
        current = current - (result[i] * diff);
    }

    printf("ESP32 Target:\t");
    for (int i = 0; i < 8; i++) {
        printf("%08x", result[i]);
    }
    printf("\n");
}

/**
 * SOURCE: https://github.com/kanoi/cgminer/blob/master/cgminer.c#L7843 - set_target
 *
 * truediffone == 0x00000000FFFF0000000000000000000000000000000000000000000000000000
 * Generate a 256 bit binary LE target by cutting up diff into 64 bit sized
 * portions or vice versa.
 *
 * calc_target64 === cgminer set_target
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
     * We have to start from bits192, because truediffone is 0x00000000FFFF....
     * there's a predetermined offset of 4 bytes set to 0 - so even when doing the 32-bit equivalent operations of
     * set_target, we must also start at bits192 // offset 24 in target. Both account for bits 224 through 256
     * of truediffone being 0. After that, we can proceed with 32 bit segments.
     *
     * It's something along these lines, I'm tired...
     *
     * ALSO: This is a psuedo-32 bit algorithm...we're using doubles all over the place (aka 64 bits).
     */
    double segments[6] = {bits192, bits160, bits128, bits96, bits64, bits32};
    uint8_t segment_offsets_in_target[6] = {24, 20, 16, 12, 8, 4};

    d32 = truediffone;
    d32 /= diff;

    for (int i = 0; i < 6; i++) {
        dcut32 = d32 / segments[i];
        h32 = dcut32;
        data32 = (uint32_t *)(target + segment_offsets_in_target[i]);
        // *data32 = htole32(h32);
        *data32 = h32;
        dcut32 = h32;
        dcut32 *= segments[i];
        d32 -= dcut32;
    }

    h32 = d32;
    data32 = (uint32_t *)(target);
    // *data32 = htole32(h32);
    *data32 = h32;
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
    bool sig_found = false;
    for (int i = 31; i >= 0; i--) {
        if (target[i] > 0 && !sig_found) {
            // Retain the upper bound
            mask[i] = target[i];
            sig_found = true;
        } else if (target[i] > 0 && sig_found) {
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

    ESP32_calc_target(diff);

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