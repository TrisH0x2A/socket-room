#include "../include/messages.h"
#include "../include/db.h"
#include "../vendor/sqlite3.h"
#include <stdio.h>
#include <string.h>

int messages_insert(int room_id, int user_id, const char *text) {
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO messages (room_id, user_id, message) "
                      "VALUES (?, ?, ?);";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return -1;
    sqlite3_bind_int(stmt, 1, room_id);
    sqlite3_bind_int(stmt, 2, user_id);
    sqlite3_bind_text(stmt, 3, text, -1, SQLITE_STATIC);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE)
        return -1;
    return (int)sqlite3_last_insert_rowid(g_db);
}

int messages_history(int room_id, int limit, message_t msgs[]) {
    sqlite3_stmt *stmt;
    const char *sql =
        "SELECT m.id, m.room_id, m.user_id, u.username, m.message, m.created_at, u.color "
        "FROM messages m JOIN users u ON m.user_id = u.id "
        "WHERE m.room_id = ? "
        "ORDER BY m.id DESC LIMIT ? ;";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return 0;
    sqlite3_bind_int(stmt, 1, room_id);
    sqlite3_bind_int(stmt, 2, limit);
    int count = 0;
    message_t tmp[MAX_HISTORY];
    while (sqlite3_step(stmt) == SQLITE_ROW && count < MAX_HISTORY) {
        tmp[count].id = sqlite3_column_int(stmt, 0);
        tmp[count].room_id = sqlite3_column_int(stmt, 1);
        tmp[count].user_id = sqlite3_column_int(stmt, 2);
        strncpy(tmp[count].username, (const char *)sqlite3_column_text(stmt, 3), 63);
        tmp[count].username[63] = '\0';
        strncpy(tmp[count].message, (const char *)sqlite3_column_text(stmt, 4), 4095);
        tmp[count].message[4095] = '\0';
        tmp[count].created_at = (long)sqlite3_column_int64(stmt, 5);
        strncpy(tmp[count].color, (const char *)sqlite3_column_text(stmt, 6), 7);
        tmp[count].color[7] = '\0';
        count++;
    }
    sqlite3_finalize(stmt);
    for (int i = 0; i < count; i++)
        msgs[i] = tmp[count - 1 - i];
    return count;
}

int messages_delete(int msg_id, int user_id) {
    sqlite3_stmt *stmt;
    const char *sql = "DELETE FROM messages WHERE id = ? AND user_id = ?;";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return -1;
    sqlite3_bind_int(stmt, 1, msg_id);
    sqlite3_bind_int(stmt, 2, user_id);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE)
        return -1;
    if (sqlite3_changes(g_db) == 0)
        return -1;
    return 0;
}
