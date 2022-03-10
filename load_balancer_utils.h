#ifndef LOAD_BALANCER_UTILS_H_
#define LOAD_BALANCER_UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "load_balancer.h"

void insert_server(load_balancer *main, int server_id, int pos, u_int hash);

void remove_server(load_balancer *main, int server_id, int pos);

void remap_objects_insert(load_balancer *main, int next_pos);

void remap_objects_remove(load_balancer *main, int prev_pos, int next_pos,
                          hashring_t old_server);

void resize_server_array(load_balancer *main);

void resize_hashring(load_balancer *main);

int binary_search_object(load_balancer *main, u_int object_hash);

int binary_search_server(load_balancer *main, u_int server_hash, int server_id);

#endif  // LOAD_BALANCER_UTILS_H_
