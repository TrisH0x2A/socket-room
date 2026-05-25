#include "../include/server.h"
#include "../include/auth.h"
#include "../include/config.h"
#include "../include/router.h"
#include "../include/websocket.h"
#include "../vendor/mongoose.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static volatile int s_running = 1;

static void ev_handler(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *)ev_data;
        if (mg_match(hm->uri, mg_str("/ws"), NULL)) {
            char token[65] = {0};
            if (hm->query.buf && hm->query.len > 0) {
                char qs[256];
                size_t qlen = hm->query.len < 255 ? hm->query.len : 255;
                memcpy(qs, hm->query.buf, qlen);
                qs[qlen] = '\0';
                char *p = strstr(qs, "token=");
                if (p) {
                    p += 6;
                    int i = 0;
                    while (*p && *p != '&' && i < 64)
                        token[i++] = *p++;
                    token[i] = '\0';
                }
            }
            if (!token[0]) {
                struct mg_str *ck = mg_http_get_header(hm, "Cookie");
                if (ck) {
                    char cs[512];
                    size_t l = ck->len < 511 ? ck->len : 511;
                    memcpy(cs, ck->buf, l);
                    cs[l] = '\0';
                    auth_cookie_token(cs, token);
                }
            }
            char *tok_copy = (char *)malloc(65);
            if (tok_copy) {
                strncpy(tok_copy, token, 64);
                tok_copy[64] = '\0';
            }
            c->fn_data = tok_copy;
            mg_ws_upgrade(c, hm, NULL);
            return;
        }
        if (router_dispatch(c, hm))
            return;
        struct mg_http_serve_opts opts = {.root_dir = SR_STATIC,
                                          .ssi_pattern = NULL,
                                          .page404 = SR_STATIC "/index.html",
                                          .extra_headers = "Cache-Control: no-cache\r\n"};
        mg_http_serve_dir(c, hm, &opts);
    } else if (ev == MG_EV_WS_OPEN || ev == MG_EV_WS_MSG || ev == MG_EV_CLOSE) {
        ws_handle(c, ev, ev_data);
    }
}

void server_run(void) {
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    char listen_addr[64];
    snprintf(listen_addr, sizeof(listen_addr), "http://0.0.0.0:%s", SR_PORT);
    struct mg_connection *nc = mg_http_listen(&mgr, listen_addr, ev_handler, NULL);
    if (!nc) {
        fprintf(stderr, "[server] Failed to bind on %s\n", listen_addr);
        return;
    }
    fprintf(stdout, "[server] Listening on %s\n", listen_addr);
    fprintf(stdout, "[server] Static files from: %s\n", SR_STATIC);
    while (s_running) {
        mg_mgr_poll(&mgr, 100);
    }
    mg_mgr_free(&mgr);
    fprintf(stdout, "[server] Stopped.\n");
}

void server_stop(void) { s_running = 0; }
