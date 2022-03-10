#include <stdlib.h>
#include <string.h>

#include "server.h"
#include "utils.h"

#define SERVER_HT_SIZE 100

server_memory* init_server_memory() {
	server_memory *server = (server_memory *)malloc(sizeof(server_memory));
	DIE(!server, "server memory malloc failed");

	// Allocate the memory of the server (is be a hashtable)
	server->hashtable = ht_create(SERVER_HT_SIZE, hash_function_string,
								  compare_function_strings);

	return server;
}

void server_store(server_memory* server, char* key, char* value) {
	unsigned int key_size = strlen(key) + 1;
	unsigned int value_size = strlen(value) + 1;

	ht_put(server->hashtable, key, key_size, value, value_size);

	// If the load factor is too big, resize the hashtable
	double load_factor = 1.0 * server->hashtable->size / server->hashtable->hmax;
	if (load_factor > 0.75)
		ht_resize_string(&server->hashtable);
}

void server_remove(server_memory* server, char* key) {
	ht_remove_entry(server->hashtable, key);
}

char* server_retrieve(server_memory* server, char* key) {
	char *value = ht_get(server->hashtable, key);
	return value;
}

void free_server_memory(server_memory* server) {
	if (server == NULL)
		return;
	if (server->hashtable != NULL)
		ht_free(server->hashtable);
	free(server);
}
