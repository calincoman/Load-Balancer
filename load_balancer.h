#ifndef LOAD_BALANCER_H_
#define LOAD_BALANCER_H_

#include "server.h"
#include "utils.h"

typedef unsigned int u_int;

struct load_balancer;
typedef struct load_balancer load_balancer;

struct hashring_t;
typedef struct hashring_t hashring_t;

// hashring_t type represents a server (a replica) on the hashring
struct hashring_t {
    // the hash of a server (a replica) from the hashring
    u_int hash;
    // ID of the server (the three replicas will have the same ID)
    int id;
};

struct load_balancer {
    // Array of pointers to elements of type server_memory
    // On i-th position, we have a pointer to the memory of the server with ID = i
    server_memory **servers;
    // Array of hashring_t type elements (the hashring)
    hashring_t *hashring;
    // Maximum server ID
    int max_server_id;
    // Length of the hashring
    int hashring_len;
    // Maximum length of the hashring
    int max_hr_len;
};

unsigned int hash_function_key(void *a);

load_balancer* init_load_balancer();

void free_load_balancer(load_balancer* main);

/**
 * load_store() - Stores the key-value pair inside the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: Key represented as a string.
 * @arg3: Value represented as a string.
 * @arg4: This function will RETURN via this parameter 
 *        the server ID which stores the object.
 *
 * The load balancer will use Consistent Hashing to distribute the 
 * load across the servers. The chosen server ID will be returned 
 * using the last parameter.
 */
void loader_store(load_balancer* main, char* key, char* value, int* server_id);

/**
 * load_retrieve() - Gets a value associated with the key.
 * @arg1: Load balancer which distributes the work.
 * @arg2: Key represented as a string.
 * @arg3: This function will RETURN the server ID 
          which stores the value via this parameter.
 *
 * The load balancer will search for the server which should posess the 
 * value associated to the key. The server will return NULL in case 
 * the key does NOT exist in the system.
 */
char* loader_retrieve(load_balancer* main, char* key, int* server_id);

/**
 * load_add_server() - Adds a new server to the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: ID of the new server.
 *
 * The load balancer will generate 3 replica TAGs and it will
 * place them inside the hash ring. The neighbor servers will 
 * distribute some the objects to the added server.
 */
void loader_add_server(load_balancer* main, int server_id);

/**
 * load_remove_server() - Removes a specific server from the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: ID of the removed server.
 *
 * The load balancer will distribute ALL objects stored on the
 * removed server and will delete ALL replicas from the hash ring.
 */
void loader_remove_server(load_balancer* main, int server_id);

#endif  // LOAD_BALANCER_H_
