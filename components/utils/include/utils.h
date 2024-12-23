#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <stdint.h>

int hex2char(uint8_t x, char *c);

size_t bin2hex(const uint8_t *buf, size_t buflen, char *hex, size_t hexlen);

uint8_t hex2val(char c);
void flip80bytes(void *dest_p, const void *src_p);
void flip32bytes(void *dest_p, const void *src_p);

size_t hex2bin(const char *hex, uint8_t *bin, size_t bin_len);

void print_hex(const uint8_t *b, size_t len, const size_t in_line, const char *prefix);

char *double_sha256(const char *hex_string);

uint8_t *double_sha256_bin(const uint8_t *data, const size_t data_len);

void single_sha256_bin(const uint8_t *data, const size_t data_len, uint8_t *dest);
void midstate_sha256_bin(const uint8_t *data, const size_t data_len, uint8_t *dest);

void swap_endian_words(const char *hex, uint8_t *output);

void reverse_bytes(uint8_t *data, size_t len);

double le256todouble(const void *target);

void prettyHex(unsigned char *buf, int len);

void to_hex_string(unsigned char *input, char output[], int len);

uint32_t flip32(uint32_t val);

#define STRATUM_DEFAULT_VERSION_MASK 0x1fffe000

#endif