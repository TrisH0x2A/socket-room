#include "../include/rooms.h"
#include "../include/db.h"
#include "../vendor/sqlite3.h"
#include <stdio.h>
#include <string.h>
static const char *DEFAULT_ROOMS[] = {"system-programming", "backend", "frontend", "chill-zone",
                                      NULL};

void rooms_seed(void) {
    sqlite3_stmt *stmt;
    const char *sql = "INSERT OR IGNORE INTO rooms (name) VALUES (?);";
    for (int i = 0; DEFAULT_ROOMS[i]; i++) {
        if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK)
            continue;
        sqlite3_bind_text(stmt, 1, DEFAULT_ROOMS[i], -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    fprintf(stdout, "[rooms] Seeded default rooms\n");
}

int rooms_list(room_t rooms[], int max) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT id, name FROM rooms ORDER BY id;";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return 0;
    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < max) {
        rooms[count].id = sqlite3_column_int(stmt, 0);
        strncpy(rooms[count].name, (const char *)sqlite3_column_text(stmt, 1), 63);
        rooms[count].name[63] = '\0';
        count++;
    }
    sqlite3_finalize(stmt);
    return count;
}

int rooms_find_by_name(const char *name) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT id FROM rooms WHERE name = ? LIMIT 1;";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return -1;
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    int id = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        id = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return id;
}

int rooms_find_by_id(int id, room_t *r) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT id, name FROM rooms WHERE id = ? LIMIT 1;";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return -1;
    sqlite3_bind_int(stmt, 1, id);
    int rc = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        r->id = sqlite3_column_int(stmt, 0);
        strncpy(r->name, (const char *)sqlite3_column_text(stmt, 1), 63);
        r->name[63] = '\0';
        rc = 0;
    }
    sqlite3_finalize(stmt);
    return rc;
}
