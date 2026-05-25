#pragma once
#ifndef ROUTER_H
#define ROUTER_H
#include "../vendor/mongoose.h"
int router_dispatch(struct mg_connection *c, struct mg_http_message *hm);
#endif
