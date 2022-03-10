#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "LinkedList.h"
#include "utils.h"
/**
 * Creates a Singly Linked List
 * @param data_size size in bytes of the data stored in the list
*/
linked_list_t *
ll_create(unsigned int data_size)
{
    linked_list_t* ll;

    ll = malloc(sizeof(*ll));
    DIE(ll == NULL, "linked_list malloc");

    ll->head = NULL;
    ll->data_size = data_size;
    ll->size = 0;

    return ll;
}

/**
 * Inserts a new node in the list
 * @param list the list in which we want to insert
 * @param n the position in the list where the node is inserted
 * @param new_data pointer to the data that needs to be inserted
 */
void
ll_add_nth_node(linked_list_t* list, unsigned int n, const void* new_data)
{
    if (list == NULL) {
        return;
    }

    /* n >= list->size means inserting a new node at the end of the list. */
    if (n > list->size) {
        n = list->size;
    }

    ll_node_t *curr = list->head;
    ll_node_t *prev = NULL;
    while (n > 0) {
        prev = curr;
        curr = curr->next;
        --n;
    }
    ll_node_t *new_node = (ll_node_t *)malloc(sizeof(ll_node_t));
    DIE(new_node == NULL, "new_node malloc");
    new_node->data = malloc(list->data_size);
    DIE(new_node->data == NULL, "new_node->data malloc");
    memcpy(new_node->data, new_data, list->data_size);

    new_node->next = curr;
    if (prev == NULL) {
        list->head = new_node;
    } else {
        prev->next = new_node;
    }

    list->size++;
}

/**
 * Removes node from specified position in the list
 * @param list the list from which we want to remove a node
 * @param n position of the node to be removed
 */
ll_node_t *
ll_remove_nth_node(linked_list_t* list, unsigned int n)
{
    ll_node_t *prev, *curr;

    if (list == NULL) {
        return NULL;
    }

    if (list->head == NULL) { /* The list is empty */
        return NULL;
    }

    /* n >= list->size - 1 means removing the last node of the list. */
    if (n > list->size - 1) {
        n = list->size - 1;
    }

    curr = list->head;
    prev = NULL;
    while (n > 0) {
        prev = curr;
        curr = curr->next;
        --n;
    }

    if (prev == NULL) {
        list->head = curr->next;
    } else {
        prev->next = curr->next;
    }

    list->size--;

    return curr;
}

/**
 * Returns number of nodes of given Linked List
 * @param list the list
 */
unsigned int
ll_get_size(linked_list_t* list)
{
    if (list == NULL) {
        return -1;
    }

    return list->size;
}

/**
 * Frees memory of data stored in the Linked List and the list itself
 * @param pp_list pointer to the list we want to free
 */
void
ll_free(linked_list_t** pp_list)
{
    ll_node_t* currNode;

    if (pp_list == NULL || *pp_list == NULL) {
        return;
    }

    while (ll_get_size(*pp_list) > 0) {
        currNode = ll_remove_nth_node(*pp_list, 0);
        free(currNode->data);
        free(currNode);
    }

    free(*pp_list);
    *pp_list = NULL;
}

/*
 * Prints all int values stored in a Linked List
 * Works only if the link stores ONLY int values
 */
void
ll_print_int(linked_list_t* list)
{
    ll_node_t* curr;

    if (list == NULL) {
        return;
    }

    curr = list->head;
    while (curr != NULL) {
        printf("%d ", *((int*)curr->data));
        curr = curr->next;
    }

    printf("\n");
}

/*
 * Prints all strings stored in a Linked List
 * Works only if the link stores ONLY strings
 */
void
ll_print_string(linked_list_t* list)
{
    ll_node_t* curr;

    if (list == NULL) {
        return;
    }

    curr = list->head;
    while (curr != NULL) {
        printf("%s ", (char*)curr->data);
        curr = curr->next;
    }

    printf("\n");
}
