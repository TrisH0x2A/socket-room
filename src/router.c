#include "../include/router.h"
#include "../include/auth.h"
#include "../include/messages.h"
#include "../include/rooms.h"
#include "../include/users.h"
#include "../include/utils.h"
#include "../vendor/mongoose.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    uint8_t ip[16];
    long window_start;
    int hits;
} ip_tracker_t;

static ip_tracker_t s_ip_trackers[256];

static int check_rate_limit(struct mg_connection *c) {
    long now = (long)time(NULL);
    for (int i = 0; i < 256; i++) {
        if (s_ip_trackers[i].window_start == 0) {
            memcpy(s_ip_trackers[i].ip, c->rem.addr.ip, 16);
            s_ip_trackers[i].window_start = now;
            s_ip_trackers[i].hits = 1;
            return 1;
        }
        if (memcmp(s_ip_trackers[i].ip, c->rem.addr.ip, 16) == 0) {
            if (now - s_ip_trackers[i].window_start > 60) {
                s_ip_trackers[i].window_start = now;
                s_ip_trackers[i].hits = 1;
                return 1;
            }
            s_ip_trackers[i].hits++;
            if (s_ip_trackers[i].hits > 5)
                return 0;
            return 1;
        }
    }
    memset(s_ip_trackers, 0, sizeof(s_ip_trackers));
    return 1;
}

static void json_reply(struct mg_connection *c, int code, const char *body) {
    mg_http_reply(c, code,
                  "Content-Type: application/json\r\n"
                  "Access-Control-Allow-Origin: *\r\n"
                  "Access-Control-Allow-Credentials: true\r\n",
                  "%s", body);
}

static void ok_json(struct mg_connection *c, const char *body) { json_reply(c, 200, body); }

static void err_json(struct mg_connection *c, int code, const char *msg) {
    char buf[512], esc[256];
    json_escape(msg, esc, sizeof(esc));
    snprintf(buf, sizeof(buf), "{\"error\":\"%s\"}", esc);
    json_reply(c, code, buf);
}

static int get_request_token(struct mg_http_message *hm, char token[65]) {
    struct mg_str *cookie_hdr = mg_http_get_header(hm, "Cookie");
    if (cookie_hdr && cookie_hdr->buf) {
        char cookie_str[512];
        size_t clen = cookie_hdr->len < 511 ? cookie_hdr->len : 511;
        memcpy(cookie_str, cookie_hdr->buf, clen);
        cookie_str[clen] = '\0';
        if (auth_cookie_token(cookie_str, token))
            return 1;
    }
    return 0;
}

static void read_body(struct mg_http_message *hm, char *out, size_t out_sz) {
    size_t blen = hm->body.len < out_sz - 1 ? hm->body.len : out_sz - 1;
    memcpy(out, hm->body.buf, blen);
    out[blen] = '\0';
}

static int body_get_str(const char *body, const char *key, char *out, size_t out_sz) {
    char pat[128];
    snprintf(pat, sizeof(pat), "\"%s\":\"", key);
    const char *p = strstr(body, pat);
    if (!p)
        return -1;
    p += strlen(pat);
    size_t i = 0;
    while (*p && *p != '"' && i < out_sz - 1) {
        if (*p == '\\')
            p++;
        out[i++] = *p++;
    }
    out[i] = '\0';
    return (i > 0) ? 0 : -1;
}

static void handle_options(struct mg_connection *c) {
    mg_http_reply(c, 204,
                  "Access-Control-Allow-Origin: *\r\n"
                  "Access-Control-Allow-Methods: GET,POST,OPTIONS\r\n"
                  "Access-Control-Allow-Headers: Content-Type,Authorization\r\n"
                  "Access-Control-Allow-Credentials: true\r\n",
                  "");
}

static void handle_register(struct mg_connection *c, struct mg_http_message *hm) {
    if (!check_rate_limit(c)) {
        err_json(c, 429, "Too Many Requests");
        return;
    }
    char body[1024], username[64], password[256];
    read_body(hm, body, sizeof(body));
    if (body_get_str(body, "username", username, sizeof(username)) != 0 ||
        body_get_str(body, "password", password, sizeof(password)) != 0) {
        err_json(c, 400, "Missing username or password");
        return;
    }
    if (auth_register(username, password) != 0) {
        err_json(c, 409, "Username already taken or invalid");
        return;
    }
    ok_json(c, "{\"ok\":true}");
}

static void handle_login(struct mg_connection *c, struct mg_http_message *hm) {
    if (!check_rate_limit(c)) {
        err_json(c, 429, "Too Many Requests");
        return;
    }
    char body[1024], username[64], password[256];
    read_body(hm, body, sizeof(body));
    if (body_get_str(body, "username", username, sizeof(username)) != 0 ||
        body_get_str(body, "password", password, sizeof(password)) != 0) {
        err_json(c, 400, "Missing username or password");
        return;
    }
    char token[65];
    int uid = auth_login(username, password, token);
    if (uid < 0) {
        err_json(c, 401, "Invalid credentials");
        return;
    }
    char esc_user[128];
    json_escape(username, esc_user, sizeof(esc_user));
    char body_out[320];
    snprintf(body_out, sizeof(body_out),
             "{\"ok\":true,\"username\":\"%s\",\"user_id\":%d,\"token\":\"%s\"}", esc_user, uid,
             token);
    char hdrs[512];
    snprintf(hdrs, sizeof(hdrs),
             "Content-Type: application/json\r\n"
             "Set-Cookie: sr_token=%s; Path=/; HttpOnly; SameSite=Lax\r\n"
             "Access-Control-Allow-Origin: *\r\n"
             "Access-Control-Allow-Credentials: true\r\n",
             token);
    mg_http_reply(c, 200, hdrs, "%s", body_out);
}

static void handle_logout(struct mg_connection *c, struct mg_http_message *hm) {
    char token[65];
    if (!get_request_token(hm, token)) {
        err_json(c, 401, "Not logged in");
        return;
    }
    auth_logout(token);
    mg_http_reply(c, 200,
                  "Content-Type: application/json\r\n"
                  "Set-Cookie: sr_token=; Path=/; Max-Age=0\r\n",
                  "%s", "{\"ok\":true}");
}

static void handle_change_password(struct mg_connection *c, struct mg_http_message *hm) {
    char token[65];
    if (!get_request_token(hm, token)) {
        err_json(c, 401, "Unauthorized");
        return;
    }
    char body[1024], old_password[256], new_password[256];
    read_body(hm, body, sizeof(body));
    if (body_get_str(body, "old_password", old_password, sizeof(old_password)) != 0 ||
        body_get_str(body, "new_password", new_password, sizeof(new_password)) != 0) {
        err_json(c, 400, "Missing old_password or new_password");
        return;
    }
    if (auth_change_password(token, old_password, new_password) != 0) {
        err_json(c, 400, "Failed to change password (invalid old password or error)");
        return;
    }
    ok_json(c, "{\"ok\":true}");
}

static void handle_change_username(struct mg_connection *c, struct mg_http_message *hm) {
    char token[65];
    if (!get_request_token(hm, token)) {
        err_json(c, 401, "Unauthorized");
        return;
    }
    char body[1024], current_password[256], new_username[256];
    read_body(hm, body, sizeof(body));
    if (body_get_str(body, "current_password", current_password, sizeof(current_password)) != 0 ||
        body_get_str(body, "new_username", new_username, sizeof(new_username)) != 0) {
        err_json(c, 400, "Missing current_password or new_username");
        return;
    }
    if (strlen(new_username) < 3 || strlen(new_username) > 30) {
        err_json(c, 400, "Username must be 3-30 characters");
        return;
    }
    int rc = auth_change_username(token, current_password, new_username);
    if (rc == -1) {
        err_json(c, 400, "Invalid current password");
        return;
    } else if (rc == -2) {
        err_json(c, 409, "Username already taken");
        return;
    } else if (rc != 0) {
        err_json(c, 500, "Internal error");
        return;
    }
    ok_json(c, "{\"ok\":true}");
}

static void handle_me(struct mg_connection *c, struct mg_http_message *hm) {
    char token[65];
    if (!get_request_token(hm, token)) {
        err_json(c, 401, "Unauthorized");
        return;
    }
    int uid = auth_validate(token);
    if (uid < 0) {
        err_json(c, 401, "Unauthorized");
        return;
    }
    user_t u;
    if (users_find_by_id(uid, &u) != 0) {
        err_json(c, 404, "User not found");
        return;
    }
    char esc[128];
    json_escape(u.username, esc, sizeof(esc));
    char buf[320];
    snprintf(buf, sizeof(buf),
             "{\"id\":%d,\"username\":\"%s\",\"color\":\"%s\",\"created_at\":%ld,\"token\":\"%s\"}",
             u.id, esc, u.color, u.created_at, token);
    ok_json(c, buf);
}

static void handle_rooms(struct mg_connection *c, struct mg_http_message *hm) {
    char token[65];
    if (!get_request_token(hm, token)) {
        err_json(c, 401, "Unauthorized");
        return;
    }
    if (auth_validate(token) < 0) {
        err_json(c, 401, "Unauthorized");
        return;
    }
    room_t rooms[MAX_ROOMS];
    int count = rooms_list(rooms, MAX_ROOMS);
    char buf[2048];
    int pos = 0;
    pos += snprintf(buf + pos, sizeof(buf) - pos, "[");
    for (int i = 0; i < count; i++) {
        char esc[128];
        json_escape(rooms[i].name, esc, sizeof(esc));
        pos += snprintf(buf + pos, sizeof(buf) - pos, "%s{\"id\":%d,\"name\":\"%s\"}", i ? "," : "",
                        rooms[i].id, esc);
    }
    pos += snprintf(buf + pos, sizeof(buf) - pos, "]");
    ok_json(c, buf);
}

static void handle_messages(struct mg_connection *c, struct mg_http_message *hm) {
    char token[65];
    if (!get_request_token(hm, token)) {
        err_json(c, 401, "Unauthorized");
        return;
    }
    if (auth_validate(token) < 0) {
        err_json(c, 401, "Unauthorized");
        return;
    }
    char room_id_str[16];
    if (get_query_param(hm->query.buf, "room_id", room_id_str, sizeof(room_id_str)) != 0) {
        err_json(c, 400, "Missing room_id");
        return;
    }
    int room_id = atoi(room_id_str);
    if (room_id <= 0) {
        err_json(c, 400, "Invalid room_id");
        return;
    }
    message_t msgs[MAX_HISTORY];
    int count = messages_history(room_id, MAX_HISTORY, msgs);
    char *buf = (char *)malloc(count * 8600 + 64);
    if (!buf) {
        err_json(c, 500, "OOM");
        return;
    }
    int pos = 0;
    pos += sprintf(buf + pos, "[");
    for (int i = 0; i < count; i++) {
        char msg_esc[8192], user_esc[128];
        json_escape(msgs[i].message, msg_esc, sizeof(msg_esc));
        json_escape(msgs[i].username, user_esc, sizeof(user_esc));
        pos += sprintf(buf + pos,
                       "%s{\"id\":%d,\"room_id\":%d,\"user_id\":%d,"
                       "\"username\":\"%s\",\"color\":\"%s\",\"text\":\"%s\",\"created_at\":%ld}",
                       i ? "," : "", msgs[i].id, msgs[i].room_id, msgs[i].user_id, user_esc,
                       msgs[i].color, msg_esc, msgs[i].created_at);
    }
    pos += sprintf(buf + pos, "]");
    ok_json(c, buf);
    free(buf);
}

int router_dispatch(struct mg_connection *c, struct mg_http_message *hm) {
    if (mg_match(hm->method, mg_str("OPTIONS"), NULL)) {
        handle_options(c);
        return 1;
    }
    if (mg_match(hm->uri, mg_str("/api/register"), NULL) &&
        mg_match(hm->method, mg_str("POST"), NULL)) {
        handle_register(c, hm);
        return 1;
    }
    if (mg_match(hm->uri, mg_str("/api/login"), NULL) &&
        mg_match(hm->method, mg_str("POST"), NULL)) {
        handle_login(c, hm);
        return 1;
    }
    if (mg_match(hm->uri, mg_str("/api/logout"), NULL) &&
        mg_match(hm->method, mg_str("POST"), NULL)) {
        handle_logout(c, hm);
        return 1;
    }
    if (mg_match(hm->uri, mg_str("/api/change_password"), NULL) &&
        mg_match(hm->method, mg_str("POST"), NULL)) {
        handle_change_password(c, hm);
        return 1;
    }
    if (mg_match(hm->uri, mg_str("/api/change_username"), NULL) &&
        mg_match(hm->method, mg_str("POST"), NULL)) {
        handle_change_username(c, hm);
        return 1;
    }
    if (mg_match(hm->uri, mg_str("/api/me"), NULL) && mg_match(hm->method, mg_str("GET"), NULL)) {
        handle_me(c, hm);
        return 1;
    }
    if (mg_match(hm->uri, mg_str("/api/rooms"), NULL) &&
        mg_match(hm->method, mg_str("GET"), NULL)) {
        handle_rooms(c, hm);
        return 1;
    }
    if (mg_match(hm->uri, mg_str("/api/messages"), NULL) &&
        mg_match(hm->method, mg_str("GET"), NULL)) {
        handle_messages(c, hm);
        return 1;
    }
    if (mg_match(hm->uri, mg_str("/api/change_password"), NULL) &&
        mg_match(hm->method, mg_str("POST"), NULL)) {
        handle_change_password(c, hm);
        return 1;
    }
    return 0;
}
