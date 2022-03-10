#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

#include "Hashtable.h"

#define MAX_BUCKET_SIZE 64

/**
 * Compares keys of type int
 */
int
compare_function_ints(void *a, void *b)
{
	int int_a = *((int *)a);
	int int_b = *((int *)b);

	if (int_a == int_b) {
		return 0;
	} else if (int_a < int_b) {
		return -1;
	} else {
		return 1;
	}
}

/**
 * Compares keys of type char
 */
int
compare_function_strings(void *a, void *b)
{
	char *str_a = (char *)a;
	char *str_b = (char *)b;

	return strcmp(str_a, str_b);
}

/*
 * Hash function for int
 */
unsigned int
hash_function_int(void *a)
{
	/*
	 * Credits: https://stackoverflow.com/a/12996028/7883884
	 */
	unsigned int uint_a = *((unsigned int *)a);

	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = (uint_a >> 16u) ^ uint_a;
	return uint_a;
}

/*
 * Hash function for char
 */
unsigned int
hash_function_string(void *a)
{
	/*
	 * Credits: http://www.cse.yorku.ca/~oz/hash.html
	 */
	unsigned char *puchar_a = (unsigned char*) a;
	unsigned long hash = 5381;
	int c;

	while ((c = *puchar_a++))
		hash = ((hash << 5u) + hash) + c; /* hash * 33 + c */

	return hash;
}

/**
 * Allocs and initializes a hashtable
 * @param hmax initial number of buckets
 * @param hash_function hash function used
 * @param compare_function compare function used
 */
hashtable_t *
ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
		int (*compare_function)(void*, void*))
{
	hashtable_t *ht = (hashtable_t *)malloc(sizeof(hashtable_t));
	DIE(ht == NULL, "malloc() failed\n");

	ht->buckets = (linked_list_t **)malloc(hmax * sizeof(linked_list_t *));
	DIE(ht->buckets == NULL, "malloc() failed");

	for (unsigned int i = 0; i < hmax; ++i)
		ht->buckets[i] = ll_create(sizeof(struct info));

	ht->size = 0;
	ht->hmax = hmax;
	ht->compare_function = compare_function;
	ht->hash_function = hash_function;

	return ht;
}

/**
 * Inserts an object (key, value) in the hashtable
 * @param ht the hashtable
 * @param key pointer to key
 * @param key_size key size in bytes
 * @param value pointer to the data
 * @param value_size data size in bytes
 */
void
ht_put(hashtable_t *ht, void *key, unsigned int key_size,
	void *value, unsigned int value_size)
{
	int index = ht->hash_function(key) % ht->hmax;

	ll_node_t *it = ht->buckets[index]->head;
	int found = 0;
	while (it != NULL) {
		struct info *node_info =(struct info *)it->data;
		if (ht->compare_function(node_info->key, key) == 0) {
			found = 1;
			memcpy(((struct info *)it->data)->value, value, value_size);
		}
		it = it->next;
	}
	if (found == 0) {
		struct info *new_info = (struct info *)malloc(sizeof(struct info));
		DIE(new_info == NULL, "malloc() failed\n");

		new_info->key = (void *)malloc(key_size);
		DIE(new_info->key == NULL, "malloc() failed\n");
		new_info->value = (void *)malloc(value_size);
		DIE(new_info->value == NULL, "malloc() failed\n");

		memcpy(new_info->key, key, key_size);
		memcpy(new_info->value, value, value_size);

		ll_add_nth_node(ht->buckets[index], ht->buckets[index]->size + 1, new_info);
		ht->size++;
		free(new_info);
	}
}
/**
 * Returns a pointer to the data matching the key in the hashtable
 * @param ht the hashtable in which we search the data
 * @param key the key
 */
void *
ht_get(hashtable_t *ht, void *key)
{
	if (ht == NULL)
		return NULL;

	int index = ht->hash_function(key) % ht->hmax;

	ll_node_t *it = ht->buckets[index]->head;
	while (it != NULL) {
		struct info *node_info = (struct info *)it->data;
		if (ht->compare_function(node_info->key, key) == 0)
			return node_info->value;
		it = it->next;
	}
	return NULL;
}

/**
 * Function which returns:
 * 	1, if the key is already in the hashtable
 * 	0, otherwise.
 * @param ht the hashtable where we search the key
 * @param key the key that we want to search
 */
int
ht_has_key(hashtable_t *ht, void *key)
{
	if (ht == NULL)
		return 0;

	int index = ht->hash_function(key) % ht->hmax;

	ll_node_t *it = ht->buckets[index]->head;
	while (it != NULL) {
		struct info *node_info = (struct info *)it->data;
		if (ht->compare_function(node_info->key, key) == 0)
			return 1;
		it = it->next;
	}
	return 0;
}

/**
 * Removes the (key, value) pair from the hashtable
 * @param ht the hashtable
 * @param key the key of the data which we want to remove
 */
void
ht_remove_entry(hashtable_t *ht, void *key)
{
	int index = ht->hash_function(key) % ht->hmax;

	ll_node_t *it = ht->buckets[index]->head;
	int cnt = 0;
	while (it != NULL) {
		struct info *node_info = (struct info *)it->data;
		if (ht->compare_function(node_info->key, key) == 0) {
			ll_node_t *removedNode = ll_remove_nth_node(ht->buckets[index], cnt);

			free(((struct info *)removedNode->data)->key);
			free(((struct info *)removedNode->data)->value);
			free(removedNode->data);
			free(removedNode);

			ht->size--;
			return;
		}
		++cnt;
		it = it->next;
	}
}
/**
 * Resizes a hashtable that contains strings
 * @param hash_table pointer to the hashtable that needs to be resized
 */
void ht_resize_string(hashtable_t **hash_table)
{
    hashtable_t *old_ht = *hash_table;
    int hmax = old_ht->hmax;

    hashtable_t *new_ht = ht_create(hmax * 2, hash_function_string,
									compare_function_strings);
	DIE(!new_ht, "hashtable resize failed");
	new_ht->hmax = 2 * hmax;

    for (int i = 0; i < hmax; ++i) {
        ll_node_t *it = old_ht->buckets[i]->head;

        while (it != NULL) {
            struct info *data = (struct info *)it->data;

			unsigned int key_size = 1 + strlen((char *)data->key);
			unsigned int value_size = 1 + strlen((char *)data->value);

			ht_put(new_ht, data->key, key_size, data->value, value_size);

            it = it->next;
        }
    }
	ht_free(old_ht);
	*hash_table = new_ht;
}

/**
 * Frees all memory used by the data in the hashtable and the hashtable itself.
 * @param ht the hashtable we want to free
 */
void
ht_free(hashtable_t *ht)
{
	DIE(ht == NULL, "Hashtable is not allocated");

	for (unsigned int i = 0; i < ht->hmax; ++i) {
		ll_node_t *it = ht->buckets[i]->head;
		while (it != NULL) {
			ll_node_t *aux = it;
			it = it->next;

			free(((struct info *)aux->data)->key);
			free(((struct info *)aux->data)->value);
			free(aux->data);
			free(aux);
		}
		free(ht->buckets[i]);
	}
	free(ht->buckets);
	free(ht);
}
/**
 * Returns number of objects stored in the hashtable 
 * @param ht the hashtable
 */
unsigned int
ht_get_size(hashtable_t *ht)
{
	if (ht == NULL)
		return 0;

	return ht->size;
}

/**
 * Returns number of buckets in a hashtable
 * @param ht the hashtable
 */
unsigned int
ht_get_hmax(hashtable_t *ht)
{
	if (ht == NULL)
		return 0;

	return ht->hmax;
}
