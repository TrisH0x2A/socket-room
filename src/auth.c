#include "../include/auth.h"
#include "../include/db.h"
#include "../include/users.h"
#include "../include/utils.h"
#include "../vendor/sqlite3.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int auth_register(const char *username, const char *password) {
    if (!username || !password || !*username || !*password)
        return -1;
    if (strlen(username) > 32 || strlen(password) > 128)
        return -1;
    user_t existing;
    if (users_find_by_username(username, &existing) == 0) {
        fprintf(stderr, "[auth] Username '%s' already exists\n", username);
        return -1;
    }
    char salt[33], hash[65];
    gen_salt(salt);
    hash_password(password, salt, hash);
    static int seeded = 0;
    if (!seeded) {
        srand(time(NULL));
        seeded = 1;
    }
    char color[8];
    snprintf(color, sizeof(color), "c%d", (rand() % 6) + 1);
    int uid = users_create(username, hash, salt, color);
    if (uid < 0) {
        fprintf(stderr, "[auth] Failed to create user '%s'\n", username);
        return -1;
    }
    fprintf(stdout, "[auth] Registered user '%s' (id=%d)\n", username, uid);
    return 0;
}

int auth_login(const char *username, const char *password, char out_token[65]) {
    user_t u;
    if (users_find_by_username(username, &u) != 0) {
        fprintf(stderr, "[auth] User '%s' not found\n", username);
        return -1;
    }
    char hash[65];
    hash_password(password, u.salt, hash);
    if (strcmp(hash, u.password_hash) != 0) {
        fprintf(stderr, "[auth] Bad password for '%s'\n", username);
        return -1;
    }
    gen_token(out_token, 32);
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO sessions (token, user_id) VALUES (?, ?);";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return -1;
    sqlite3_bind_text(stmt, 1, out_token, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, u.id);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE)
        return -1;
    fprintf(stdout, "[auth] Login OK for '%s'\n", username);
    return u.id;
}

int auth_validate(const char *token) {
    if (!token || !*token)
        return -1;
    sqlite3_stmt *stmt;
    const char *sql = "SELECT user_id FROM sessions WHERE token = ? LIMIT 1;";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return -1;
    sqlite3_bind_text(stmt, 1, token, -1, SQLITE_STATIC);
    int uid = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        uid = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return uid;
}

void auth_logout(const char *token) {
    if (!token || !*token)
        return;
    sqlite3_stmt *stmt;
    const char *sql = "DELETE FROM sessions WHERE token = ?;";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return;
    sqlite3_bind_text(stmt, 1, token, -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

int auth_cookie_token(const char *cookie_header, char out[65]) {
    if (!cookie_header)
        return 0;
    const char *p = strstr(cookie_header, "sr_token=");
    if (!p)
        return 0;
    p += 9;
    int i = 0;
    while (*p && *p != ';' && *p != ' ' && i < 64)
        out[i++] = *p++;
    out[i] = '\0';
    return i > 0 ? 1 : 0;
}

int auth_change_password(const char *token, const char *old_password, const char *new_password) {
    if (!token || !old_password || !new_password)
        return -1;
    int uid = auth_validate(token);
    if (uid < 0)
        return -1;
    user_t u;
    if (users_find_by_id(uid, &u) != 0)
        return -1;
    char old_hash[65];
    hash_password(old_password, u.salt, old_hash);
    if (strcmp(old_hash, u.password_hash) != 0) {
        fprintf(stderr, "[auth] Bad old password for '%s'\n", u.username);
        return -1;
    }
    char new_salt[33], new_hash[65];
    gen_salt(new_salt);
    hash_password(new_password, new_salt, new_hash);
    if (users_update_password(uid, new_hash, new_salt) != 0) {
        return -1;
    }
    fprintf(stdout, "[auth] Changed password for '%s'\n", u.username);
    return 0;
}

int auth_change_username(const char *token, const char *current_password,
                         const char *new_username) {
    if (!token || !current_password || !new_username)
        return -1;
    int uid = auth_validate(token);
    if (uid < 0)
        return -1;
    user_t u;
    if (users_find_by_id(uid, &u) != 0)
        return -1;
    char old_hash[65];
    hash_password(current_password, u.salt, old_hash);
    if (strcmp(old_hash, u.password_hash) != 0) {
        fprintf(stderr, "[auth] Bad password when changing username for '%s'\n", u.username);
        return -1;
    }
    if (users_update_username(uid, new_username) != 0) {
        return -2;
    }
    fprintf(stdout, "[auth] Changed username for '%s' -> '%s'\n", u.username, new_username);
    return 0;
}
