#pragma once
#ifndef MESSAGES_H
#define MESSAGES_H
#define MAX_HISTORY 50

typedef struct {
    int id;
    int room_id;
    int user_id;
    char username[64];
    char color[8];
    char message[4096];
    long created_at;
} message_t;

int messages_insert(int room_id, int user_id, const char *text);
int messages_history(int room_id, int limit, message_t msgs[]);
int messages_delete(int msg_id, int user_id);
#endif
