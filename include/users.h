#pragma once
#ifndef USERS_H
#define USERS_H

typedef struct {
    int id;
    char username[64];
    char password_hash[65];
    char salt[33];
    char color[8];
    long created_at;
} user_t;

int users_find_by_username(const char *username, user_t *u);
int users_find_by_id(int id, user_t *u);
int users_create(const char *username, const char *password_hash, const char *salt,
                 const char *color);
int users_update_password(int user_id, const char *new_hash, const char *new_salt);
int users_update_username(int user_id, const char *new_username);
#endif
