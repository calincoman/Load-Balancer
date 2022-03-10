#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

#include "load_balancer.h"
#include "load_balancer_utils.h"

#define INIT_SIZE 10000
#define REPLICA_FACTOR 100000

typedef unsigned int u_int;

unsigned int hash_function_servers(void *a) {
    unsigned int uint_a = *((unsigned int *)a);

    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = (uint_a >> 16u) ^ uint_a;
    return uint_a;
}

unsigned int hash_function_key(void *a) {
    unsigned char *puchar_a = (unsigned char *) a;
    unsigned int hash = 5381;
    int c;

    while ((c = *puchar_a++))
        hash = ((hash << 5u) + hash) + c;

    return hash;
}

load_balancer* init_load_balancer() {

    load_balancer *main_server = (load_balancer*) malloc(sizeof(load_balancer));
    DIE(!main_server, "load balancer malloc failed");

    main_server->servers = (server_memory **)malloc(INIT_SIZE
                                            * sizeof(server_memory *));
    DIE(!main_server->servers, "load balancer malloc failed");
    for (int i = 0; i < INIT_SIZE; ++i)
        main_server->servers[i] = NULL;

    // Allocate the hashring with an initial dimension
    main_server->hashring = (hashring_t *)malloc(INIT_SIZE
                                                 * sizeof(hashring_t));
    DIE(!main_server->hashring, "load balancer malloc failed");

    // Set the initial maximum dimensions
    main_server->max_server_id = INIT_SIZE;
    main_server->hashring_len = 0;
    main_server->max_hr_len = INIT_SIZE;

    return main_server;
}

void loader_store(load_balancer* main, char* key, char* value, int* server_id) {

    // Binary search the server which the object will be stored on
    u_int object_hash = hash_function_key(key);
    *server_id = binary_search_object(main, object_hash);

    // Place the object in the found server
    server_store(main->servers[*server_id], key, value);
}

char* loader_retrieve(load_balancer* main, char* key, int* server_id) {

    // Search the server which the object is stored on and return the object's value
    u_int object_hash = hash_function_key(key);
    *server_id = binary_search_object(main, object_hash);
    return server_retrieve(main->servers[*server_id], key);
}

void loader_add_server(load_balancer* main, int server_id) {

    // If a bigger ID is needed resize the array
    while (server_id >= main->max_server_id)
        resize_server_array(main);

    // Alloc memory for the new server
    main->servers[server_id] = init_server_memory();

    // Calculate the ID of the replicas of the new server
    int replica_id_1 = 1 * REPLICA_FACTOR + server_id;
    int replica_id_2 = 2 * REPLICA_FACTOR + server_id;

    // Calculate the hashes
    u_int server_hash = hash_function_servers(&server_id);
    u_int replica_hash_1 = hash_function_servers(&replica_id_1);
    u_int replica_hash_2 = hash_function_servers(&replica_id_2);

    // Search binary for every replica and insert in the hashring
    int pos = binary_search_server(main, server_hash, server_id);
    insert_server(main, server_id, pos, server_hash);
    int pos_1 = binary_search_server(main, replica_hash_1, server_id);
    insert_server(main, server_id, pos_1, replica_hash_1);
    int pos_2 = binary_search_server(main, replica_hash_2, server_id);
    insert_server(main, server_id, pos_2, replica_hash_2);
}

void loader_remove_server(load_balancer* main, int server_id) {

    int replica_id_1 = 1 * REPLICA_FACTOR + server_id;
    int replica_id_2 = 2 * REPLICA_FACTOR + server_id;

    u_int server_hash = hash_function_servers(&server_id);
    u_int replica_hash_1 = hash_function_servers(&replica_id_1);
    u_int replica_hash_2 = hash_function_servers(&replica_id_2);

    int pos = binary_search_server(main, server_hash, server_id);
    remove_server(main, server_id, pos);
    int pos_1 = binary_search_server(main, replica_hash_1, server_id);
    remove_server(main, server_id, pos_1);
    int pos_2 = binary_search_server(main, replica_hash_2, server_id);
    remove_server(main, server_id, pos_2);
}

void free_load_balancer(load_balancer* main) {
    for (int i = 0; i < main->max_server_id; ++i)
        free_server_memory(main->servers[i]);
    free(main->servers);

    free(main->hashring);
    free(main);
}
