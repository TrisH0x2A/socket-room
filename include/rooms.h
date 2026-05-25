#pragma once
#ifndef ROOMS_H
#define ROOMS_H
#define MAX_ROOMS 16

typedef struct {
    int id;
    char name[64];
} room_t;

void rooms_seed(void);
int rooms_list(room_t rooms[], int max);
int rooms_find_by_name(const char *name);
int rooms_find_by_id(int id, room_t *r);
#endif
