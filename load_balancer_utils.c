#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "load_balancer.h"
#include "load_balancer_utils.h"
#include "utils.h"

/**
 * Inserts a server (a replica) in the hashring
 * @param main the load balancer, in which we insert the server
 * @param server_id ID-ul of the server (not the replica) which is added
 * @param pos position in the hashring of the server that will be added
 * @param hash the hash of the server (the replica) which is added
 */
void insert_server(load_balancer *main, int server_id, int pos, u_int hash)
{
    // Increase hashring size and resize if necessary
    main->hashring_len++;
    if (main->hashring_len > main->max_hr_len)
        resize_hashring(main);

    // Shift hashring with 1 position to the right
    for (int i = main->hashring_len - 1; i > pos; --i)
        main->hashring[i] = main->hashring[i - 1];

    // Put server data on that position
    main->hashring[pos].hash = hash;
    main->hashring[pos].id = server_id;

    // Redistribute objects stored on the server after the one just added
    int next_pos = pos + 1;
    if (main->hashring_len > 0)
        remap_objects_insert(main, next_pos);
}

/**
 * Removes a server (a replica) from the hashring
 * @param main the load balancer, where the server is deleted from
 * @param server_id ID of the server which is removed
 * @param pos position of the server that needs to be removed
 */
void remove_server(load_balancer *main, int server_id, int pos)
{
    int prev_pos = pos - 1, next_pos = pos + 1;

    // Get the ID of the server situated after the one we want to remove
    // Because of the circular structure, the server after the last one is the one
    // on position 0
    int next_server_id = 0;
    if (next_pos == main->hashring_len)
        next_server_id = main->hashring[0].id;
    else
        next_server_id = main->hashring[next_pos].id;

    // Remap (redistribute) the objects before removing the server
    if (server_id != next_server_id)
        remap_objects_remove(main, prev_pos, next_pos, main->hashring[pos]);

    for (int i = pos; i < main->hashring_len - 1; ++i)
        main->hashring[i] = main->hashring[i + 1];
    if (main->hashring_len > 0)
        main->hashring_len--;
    else
        fprintf(stderr, "there are no servers left to be removed\n");
}

/**
 * Remaps the objects on the servers in case a new server is added
 * @param main the load balancer we are working on
 * @param next_pos hashring position of the server after the newly added server
 */
void remap_objects_insert(load_balancer *main, int next_pos)
{
    int next_id = 0;

    // Get the ID of the server situated after the one we want to remove
    // Because of the circular structure, the server after the last one is the one
    // on position 0
    if (next_pos == main->hashring_len)
        next_id = main->hashring[0].id;
    else
        next_id = main->hashring[next_pos].id;

    hashtable_t *old_ht = main->servers[next_id]->hashtable;

    // Remap (if we have to) every object from the server after the newly added one
    for (u_int i = 0; i < old_ht->hmax; ++i) {
        ll_node_t *it = old_ht->buckets[i]->head;

        // Iterate through the buckets
        while (it != NULL) {
            ll_node_t *next = it->next;
            struct info *obj = (struct info *)it->data;
            u_int obj_hash = hash_function_key(obj->key);

            // We search on what server the current object should be stored
            int new_id = binary_search_object(main, obj_hash);

            if (new_id != next_id) {
                server_store(main->servers[new_id], obj->key, obj->value);
                server_remove(main->servers[next_id], obj->key);
            }

            it = next;
        }
    }
}

/**
 * Remaps the objects in case a server is removed
 * @param main the load balancer we are working with
 * @param prev_pos position of the server before the removed one
 * @param nest_pos position of the server after the removed one
 * @param old_server info of the removed server
 */
void remap_objects_remove(load_balancer *main, int prev_pos, int next_pos,
                          hashring_t old_server)
{
    u_int prev_hash = 0;
    u_int curr_hash = old_server.hash;
    int curr_id = old_server.id;
    int next_id = 0;

    // Find out the hash of the server before the removed one
    if (prev_pos == -1)
        prev_hash = 0;
    else
        prev_hash = main->hashring[prev_pos].hash;

    // Find out the ID of the server after the removed one
    if (next_pos == main->hashring_len)
        next_id = main->hashring[0].id;
    else
        next_id = main->hashring[next_pos].id;

    hashtable_t *old_ht = main->servers[curr_id]->hashtable;

    for (u_int i = 0; i < old_ht->hmax; ++i) {
        ll_node_t *it = old_ht->buckets[i]->head;

        // Iterate the objects stored on the removed server.
        // Remap only the objects stored on the removed server, so the ones which satify
        // the following condition
        // prev_hash < object_hash < eliminated_server_hash
        while (it != NULL) {
            ll_node_t *next = it->next;
            struct info *obj = (struct info *)it->data;
            u_int obj_hash = hash_function_key(obj->key);

            if (obj_hash > prev_hash && obj_hash <= curr_hash) {
                server_store(main->servers[next_id], obj->key, obj->value);
                server_remove(main->servers[curr_id], obj->key);
            }
            it = next;
        }
    }
}

/**
 * Resize the hashring (double its capacity)
 * @param main the load balancer that will be resized
 */
void resize_hashring(load_balancer *main)
{
    int max_hr_len = main->max_hr_len;

    hashring_t *new_hashring = (hashring_t *)realloc(main->hashring,
                                2 * max_hr_len * sizeof(hashring_t));
    DIE(!new_hashring, "hashring resize failed");
    main->hashring = new_hashring;
    main->max_hr_len = 2 * max_hr_len;
}

/**
 * Resize the server array
 * @param main the load balancer whose server array will be resized
 */
void resize_server_array(load_balancer *main)
{
    int max_server_id = main->max_server_id;

    server_memory **new_array = realloc(main->servers,
                                max_server_id * 2 * sizeof(server_memory *));
    DIE(!new_array, "server array resize failed");
    main->servers = new_array;
    main->max_server_id = max_server_id * 2;

    for (int i = max_server_id; i < max_server_id * 2; ++i)
        main->servers[i] = NULL;
}

/**
 * Use binary search to find the server which the object must be stored on and return its ID
 * @param main the load balancer we are working on
 * @param object_hash the hash of the object
 */
int binary_search_object(load_balancer *main, u_int object_hash)
{
    int hashring_len = main->hashring_len;
    int left = 0, right = hashring_len, pos = -1;

    // Special case
    if (object_hash < main->hashring[0].hash)
        return main->hashring[0].id;

    // If the object's hash is greater than the one of the last server on the hashring,
    // then it will be stored in the server from the first position of the hashring
    if (object_hash > main->hashring[hashring_len - 1].hash)
        return main->hashring[0].id;

    // The binary search finds the server whith the smallest hash greater than the hash of the object
    while (left <= right) {
        int mid = (left + right) / 2;
        if (main->hashring[mid].hash < object_hash) {
            left = mid + 1;
        } else if (main->hashring[mid].hash >= object_hash) {
                pos = mid;
                right = mid - 1;
        }
    }

    return main->hashring[pos].id;
}

/**
 * Binary searches the server after which the new server will be added (so find the
 * smallest hash greater the the hash of the server)
 * @param main the load balancer we are working on
 * @param server_hash the hash of the searched server
 * @param server_id 
 */
int binary_search_server(load_balancer *main, u_int server_hash, int server_id)
{
    int hashring_len = main->hashring_len;
    int left = 0, right = hashring_len, pos = -1;

    // Tratam cazurile speciale
    if (main->hashring_len == 0)
        return 0;

    if (server_hash < main->hashring[0].hash)
        return 0;

    // If the hash is greater than the one of the last server in the hashring
    // return last_position + 1
    if (server_hash > main->hashring[hashring_len - 1].hash)
        return hashring_len;
        
    // The binary search looks if the server is already in the hashring, and, if not
    // seaches the server with the smallest hash greater than the hash of the searched server
    // If the two hashes are equal, compare the IDs.
    while (left <= right) {
        int mid = (left + right) / 2;
        if (main->hashring[mid].hash < server_hash) {
            left = mid + 1;
        } else if (main->hashring[mid].hash > server_hash) {
                pos = mid;
                right = mid - 1;
            } else if (main->hashring[mid].hash == server_hash) {
                if (main->hashring[mid].id > server_id) {
                    pos = mid;
                    right = mid - 1;
                } else {
                    if (main->hashring[mid].id < server_id)
                        left = mid + 1;
                    else
                        return mid;
                }
            }
    }

    return pos;
}
