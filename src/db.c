#include "../include/db.h"
#include <stdio.h>
#include <stdlib.h>
sqlite3 *g_db = NULL;
static const char *SCHEMA = "PRAGMA journal_mode=WAL;"
                            "PRAGMA foreign_keys=ON;"
                            "CREATE TABLE IF NOT EXISTS users ("
                            "  id            INTEGER PRIMARY KEY AUTOINCREMENT,"
                            "  username      TEXT UNIQUE NOT NULL,"
                            "  password_hash TEXT NOT NULL,"
                            "  salt          TEXT NOT NULL,"
                            "  color         TEXT DEFAULT 'c1',"
                            "  created_at    INTEGER DEFAULT (strftime('%s','now'))"
                            ");"
                            "CREATE TABLE IF NOT EXISTS rooms ("
                            "  id   INTEGER PRIMARY KEY AUTOINCREMENT,"
                            "  name TEXT UNIQUE NOT NULL"
                            ");"
                            "CREATE TABLE IF NOT EXISTS messages ("
                            "  id         INTEGER PRIMARY KEY AUTOINCREMENT,"
                            "  room_id    INTEGER NOT NULL REFERENCES rooms(id),"
                            "  user_id    INTEGER NOT NULL REFERENCES users(id),"
                            "  message    TEXT NOT NULL,"
                            "  created_at INTEGER DEFAULT (strftime('%s','now'))"
                            ");"
                            "CREATE TABLE IF NOT EXISTS sessions ("
                            "  token      TEXT PRIMARY KEY,"
                            "  user_id    INTEGER NOT NULL REFERENCES users(id),"
                            "  created_at INTEGER DEFAULT (strftime('%s','now'))"
                            ");";

int db_open(const char *path) {
    int rc = sqlite3_open(path, &g_db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[db] Cannot open database '%s': %s\n", path, sqlite3_errmsg(g_db));
        return -1;
    }
    char *err = NULL;
    rc = sqlite3_exec(g_db, SCHEMA, NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[db] Schema error: %s\n", err);
        sqlite3_free(err);
        return -1;
    }
    fprintf(stdout, "[db] Opened '%s'\n", path);
    return 0;
}

void db_close(void) {
    if (g_db) {
        sqlite3_close(g_db);
        g_db = NULL;
    }
}

int db_exec(const char *sql) {
    char *err = NULL;
    int rc = sqlite3_exec(g_db, sql, NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[db] exec error: %s\n", err);
        sqlite3_free(err);
        return -1;
    }
    return 0;
}
