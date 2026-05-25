#pragma once
#ifndef WEBSOCKET_H
#define WEBSOCKET_H
#include "../vendor/mongoose.h"
void ws_handle(struct mg_connection *c, int ev, void *ev_data);
void ws_broadcast_room(int room_id, const char *json, size_t len);
#endif
