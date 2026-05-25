#pragma once
#ifndef UTILS_H
#define UTILS_H
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t state[8];
    uint64_t count;
    uint8_t buf[64];
} sha256_ctx_t;

void sha256_init(sha256_ctx_t *ctx);
void sha256_update(sha256_ctx_t *ctx, const uint8_t *data, size_t len);
void sha256_final(sha256_ctx_t *ctx, uint8_t digest[32]);
void sha256_hex(const uint8_t *data, size_t len, char out[65]);
void gen_token(char *out, size_t byte_len);
void hash_password(const char *password, const char *salt, char out[65]);
void gen_salt(char out[33]);
int get_query_param(const char *query, const char *key, char *out, size_t out_len);
size_t json_escape(const char *in, char *out, size_t out_size);
int safe_snprintf(char *buf, size_t size, const char *fmt, ...);
#endif
