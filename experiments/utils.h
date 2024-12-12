#ifndef JOBS_H
#define JOBS_H

#include <openssl/sha.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#ifdef __GNUC__
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

bool DEBUGGING = false;

/**
 * Pulled from https://github.com/kanoi/cgminer/blob/e8d49a56163419fedca385e61abf88d55d8cc30d/util.c#L880C1-L929C2
 *
 * Precomputed table that maps all possible ASCII values (0-255) to a valid hexadecimal value (0-15) or 0xff (-1)
 *
 * '0' to '9' map to 0 to 9.
 * 'A' to 'F' (uppercase) map to 10 to 15.
 * 'a' to 'f' (lowercase) map to 10 to 15.
 */
static const int hex2bin_tbl[256] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
	-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

/**
 * Pulled from https://github.com/kanoi/cgminer/blob/e8d49a56163419fedca385e61abf88d55d8cc30d/util.c#L880C1-L929C2
 */
bool hex2bin(unsigned char *p, const char *hexstr, size_t len) {
  int nibble1, nibble2;
  unsigned char idx;
  bool ret = false;

  while (*hexstr && len) {
    if (unlikely(!hexstr[1])) {
      return ret;
    }

    idx = *hexstr++;
    nibble1 = hex2bin_tbl[idx];
    idx = *hexstr++;
    nibble2 = hex2bin_tbl[idx];

    if (unlikely((nibble1 < 0) || (nibble2 < 0))) {
      return ret;
    }

    *p++ = (((unsigned char)nibble1) << 4) | ((unsigned char)nibble2);
    --len;
  }

  if (likely(len == 0 && *hexstr == 0))
    ret = true;
  return ret;
}

void double_sha256(unsigned char *input, size_t len, unsigned char output[32]) {
  unsigned char temp[SHA256_DIGEST_LENGTH];
  SHA256(input, len, temp);
  SHA256(temp, SHA256_DIGEST_LENGTH, output);
}

void debug_msg(char *msg) {
  if (DEBUGGING) {
    printf("DEBUG: %s\n", msg);
  }
}

void print_hex(unsigned char *h, size_t len) {
  for (uint8_t i = 0; i < len; ++i) {
    printf("%02x", h[i]);
  }
  printf("\n");
}
#endif // POOL_COMPONENT_H