#include "../include/websocket.h"
#include "../include/auth.h"
#include "../include/messages.h"
#include "../include/users.h"
#include "../include/utils.h"
#include "../vendor/mongoose.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_WS_CLIENTS 256

typedef struct {
    struct mg_connection *conn;
    int user_id;
    int room_id;
    char username[64];
    char color[8];
    long last_msg_ts;
    int msg_burst_count;
} ws_client_t;

static ws_client_t s_clients[MAX_WS_CLIENTS];
static int s_client_count = 0;

static ws_client_t *find_client(struct mg_connection *c) {
    for (int i = 0; i < s_client_count; i++)
        if (s_clients[i].conn == c)
            return &s_clients[i];
    return NULL;
}

static ws_client_t *add_client(struct mg_connection *c) {
    if (s_client_count >= MAX_WS_CLIENTS)
        return NULL;
    ws_client_t *cl = &s_clients[s_client_count++];
    cl->conn = c;
    cl->user_id = -1;
    cl->room_id = -1;
    cl->username[0] = '\0';
    cl->color[0] = '\0';
    cl->last_msg_ts = 0;
    cl->msg_burst_count = 0;
    return cl;
}

static void remove_client(struct mg_connection *c) {
    for (int i = 0; i < s_client_count; i++) {
        if (s_clients[i].conn == c) {
            s_clients[i] = s_clients[--s_client_count];
            return;
        }
    }
}

void ws_broadcast_room(int room_id, const char *json, size_t len) {
    for (int i = 0; i < s_client_count; i++) {
        if (s_clients[i].room_id == room_id)
            mg_ws_send(s_clients[i].conn, json, len, WEBSOCKET_OP_TEXT);
    }
}

static void send_error(struct mg_connection *c, const char *msg) {
    char buf[256];
    char escaped[256];
    json_escape(msg, escaped, sizeof(escaped));
    snprintf(buf, sizeof(buf), "{\"type\":\"error\",\"message\":\"%s\"}", escaped);
    mg_ws_send(c, buf, strlen(buf), WEBSOCKET_OP_TEXT);
}

static int json_get_str(const char *json, size_t len, const char *key, char *out, size_t out_sz) {
    char pat[128];
    snprintf(pat, sizeof(pat), "\"%s\":\"", key);
    const char *p = (const char *)memmem(json, len, pat, strlen(pat));
    if (!p)
        return -1;
    p += strlen(pat);
    size_t i = 0;
    while (*p && *p != '"' && i < out_sz - 1) {
        if (*p == '\\')
            p++; /* skip escape (simple) */
        out[i++] = *p++;
    }
    out[i] = '\0';
    return (i > 0) ? 0 : -1;
}

/* Parse a simple JSON int field: {"key":123} → value */
static int json_get_int(const char *json, size_t len, const char *key, int *out) {
    char pat[128];
    snprintf(pat, sizeof(pat), "\"%s\":", key);
    const char *p = (const char *)memmem(json, len, pat, strlen(pat));
    if (!p)
        return -1;
    p += strlen(pat);
    while (*p == ' ')
        p++;
    if (*p < '0' || *p > '9')
        return -1;
    *out = atoi(p);
    return 0;
}

void ws_handle(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_WS_OPEN) {
        const char *stored_token = (const char *)c->fn_data;
        if (!stored_token || !*stored_token) {
            send_error(c, "Not authenticated");
            c->is_draining = 1;
            return;
        }
        int uid = auth_validate(stored_token);
        if (uid < 0) {
            send_error(c, "Invalid session");
            c->is_draining = 1;
            return;
        }
        ws_client_t *cl = add_client(c);
        if (!cl) {
            c->is_draining = 1;
            return;
        }
        cl->user_id = uid;
        user_t u;
        if (users_find_by_id(uid, &u) == 0) {
            snprintf(cl->username, sizeof(cl->username), "%s", u.username);
            snprintf(cl->color, sizeof(cl->color), "%s", u.color);
        }
        fprintf(stdout, "[ws] Client connected: user=%s\n", cl->username);
    } else if (ev == MG_EV_WS_MSG) {
        struct mg_ws_message *wm = (struct mg_ws_message *)ev_data;
        ws_client_t *cl = find_client(c);
        if (!cl)
            return;
        const char *data = wm->data.buf;
        size_t dlen = wm->data.len;
        char type_buf[32];
        if (json_get_str(data, dlen, "type", type_buf, sizeof(type_buf)) != 0)
            return;
        if (strcmp(type_buf, "join") == 0) {
            int room_id = 0;
            if (json_get_int(data, dlen, "room_id", &room_id) != 0)
                return;
            cl->room_id = room_id;
            message_t history[MAX_HISTORY];
            int count = messages_history(room_id, MAX_HISTORY, history);
            for (int i = 0; i < count; i++) {
                char msg_esc[8192], user_esc[128];
                json_escape(history[i].message, msg_esc, sizeof(msg_esc));
                json_escape(history[i].username, user_esc, sizeof(user_esc));
                char pkt[8448];
                snprintf(pkt, sizeof(pkt),
                         "{\"type\":\"history\","
                         "\"id\":%d,"
                         "\"room_id\":%d,"
                         "\"user_id\":%d,"
                         "\"username\":\"%s\","
                         "\"color\":\"%s\","
                         "\"text\":\"%s\","
                         "\"created_at\":%ld}",
                         history[i].id, history[i].room_id, history[i].user_id, user_esc,
                         history[i].color, msg_esc, history[i].created_at);
                mg_ws_send(c, pkt, strlen(pkt), WEBSOCKET_OP_TEXT);
            }
        } else if (strcmp(type_buf, "message") == 0) {
            long now = (long)time(NULL);
            if (now - cl->last_msg_ts <= 2) {
                cl->msg_burst_count++;
                if (cl->msg_burst_count > 5) {
                    cl->last_msg_ts = now;
                    send_error(c, "You are sending messages too fast. Please slow down.");
                    return;
                }
            } else {
                cl->msg_burst_count = 1;
            }
            cl->last_msg_ts = now;
            int room_id = 0;
            char text[4097];
            if (json_get_int(data, dlen, "room_id", &room_id) != 0)
                return;
            if (json_get_str(data, dlen, "text", text, sizeof(text)) != 0)
                return;
            if (cl->room_id != room_id)
                cl->room_id = room_id;
            int msg_id = messages_insert(room_id, cl->user_id, text);
            if (msg_id < 0) {
                send_error(c, "Failed to save message");
                return;
            }
            char msg_esc[8192], user_esc[128];
            json_escape(text, msg_esc, sizeof(msg_esc));
            json_escape(cl->username, user_esc, sizeof(user_esc));
            char pkt[8448];
            long ts = (long)time(NULL);
            snprintf(pkt, sizeof(pkt),
                     "{\"type\":\"message\","
                     "\"id\":%d,"
                     "\"room_id\":%d,"
                     "\"user_id\":%d,"
                     "\"username\":\"%s\","
                     "\"color\":\"%s\","
                     "\"text\":\"%s\","
                     "\"created_at\":%ld}",
                     msg_id, room_id, cl->user_id, user_esc, cl->color, msg_esc, ts);
            ws_broadcast_room(room_id, pkt, strlen(pkt));
        } else if (strcmp(type_buf, "delete") == 0) {
            int msg_id = 0;
            if (json_get_int(data, dlen, "id", &msg_id) != 0)
                return;
            if (messages_delete(msg_id, cl->user_id) == 0) {
                char pkt[256];
                snprintf(pkt, sizeof(pkt), "{\"type\":\"delete\",\"id\":%d,\"room_id\":%d}", msg_id,
                         cl->room_id);
                ws_broadcast_room(cl->room_id, pkt, strlen(pkt));
            } else {
                send_error(c, "Failed to delete message");
            }
        }
    } else if (ev == MG_EV_CLOSE) {
        ws_client_t *cl = find_client(c);
        if (cl) {
            fprintf(stdout, "[ws] Client disconnected: user=%s\n", cl->username);
            remove_client(c);
        }
        if (c->fn_data) {
            free(c->fn_data);
            c->fn_data = NULL;
        }
    }
}
