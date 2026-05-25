#pragma once
#ifndef DB_H
#define DB_H
#include "../vendor/sqlite3.h"
extern sqlite3 *g_db;
int db_open(const char *path);
void db_close(void);
int db_exec(const char *sql);
#endif
