#include "../include/users.h"
#include "../include/db.h"
#include "../vendor/sqlite3.h"
#include <stdio.h>
#include <string.h>

int users_find_by_username(const char *username, user_t *u) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT id, username, password_hash, salt, color, created_at "
                      "FROM users WHERE username = ? LIMIT 1;";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return -1;
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    int rc = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        u->id = sqlite3_column_int(stmt, 0);
        strncpy(u->username, (const char *)sqlite3_column_text(stmt, 1), 63);
        u->username[63] = '\0';
        strncpy(u->password_hash, (const char *)sqlite3_column_text(stmt, 2), 64);
        u->password_hash[64] = '\0';
        strncpy(u->salt, (const char *)sqlite3_column_text(stmt, 3), 32);
        u->salt[32] = '\0';
        strncpy(u->color, (const char *)sqlite3_column_text(stmt, 4), 7);
        u->color[7] = '\0';
        u->created_at = (long)sqlite3_column_int64(stmt, 5);
        rc = 0;
    }
    sqlite3_finalize(stmt);
    return rc;
}

int users_find_by_id(int id, user_t *u) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT id, username, password_hash, salt, color, created_at "
                      "FROM users WHERE id = ? LIMIT 1;";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return -1;
    sqlite3_bind_int(stmt, 1, id);
    int rc = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        u->id = sqlite3_column_int(stmt, 0);
        strncpy(u->username, (const char *)sqlite3_column_text(stmt, 1), 63);
        u->username[63] = '\0';
        strncpy(u->password_hash, (const char *)sqlite3_column_text(stmt, 2), 64);
        u->password_hash[64] = '\0';
        strncpy(u->salt, (const char *)sqlite3_column_text(stmt, 3), 32);
        u->salt[32] = '\0';
        strncpy(u->color, (const char *)sqlite3_column_text(stmt, 4), 7);
        u->color[7] = '\0';
        u->created_at = (long)sqlite3_column_int64(stmt, 5);
        rc = 0;
    }
    sqlite3_finalize(stmt);
    return rc;
}

int users_create(const char *username, const char *password_hash, const char *salt,
                 const char *color) {
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO users (username, password_hash, salt, color) "
                      "VALUES (?, ?, ?, ?);";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "[users] prepare error: %s\n", sqlite3_errmsg(g_db));
        return -1;
    }
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password_hash, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, salt, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, color, -1, SQLITE_STATIC);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "[users] insert error: %s\n", sqlite3_errmsg(g_db));
        return -1;
    }
    return (int)sqlite3_last_insert_rowid(g_db);
}

int users_update_password(int user_id, const char *new_hash, const char *new_salt) {
    sqlite3_stmt *stmt;
    const char *sql = "UPDATE users SET password_hash = ?, salt = ? WHERE id = ?;";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "[users] prepare update error: %s\n", sqlite3_errmsg(g_db));
        return -1;
    }
    sqlite3_bind_text(stmt, 1, new_hash, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, new_salt, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, user_id);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "[users] update error: %s\n", sqlite3_errmsg(g_db));
        return -1;
    }
    return 0;
}

int users_update_username(int user_id, const char *new_username) {
    sqlite3_stmt *stmt;
    const char *sql = "UPDATE users SET username = ? WHERE id = ?;";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "[users] prepare update username error: %s\n", sqlite3_errmsg(g_db));
        return -1;
    }
    sqlite3_bind_text(stmt, 1, new_username, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, user_id);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "[users] update username error: %s\n", sqlite3_errmsg(g_db));
        return -1;
    }
    return 0;
}
