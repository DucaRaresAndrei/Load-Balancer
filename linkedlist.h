/* Copyright 2023 Duca Andrei-Rares, 311CA */
#ifndef LINKEDLIST_H_
#define LINKEDLIST_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

typedef struct ll_node_t
{
    void* data;
    struct ll_node_t* next;
} ll_node_t;

typedef struct linked_list_t
{
    ll_node_t* head;
    unsigned int data_size;
    unsigned int size;
} linked_list_t;

/*
  * Creates the required list.
*/
linked_list_t *ll_create(unsigned int data_size);

/*
  * Based on the data sent through the new_data pointer, a new nodeis created which
  is added to the nth position of the list represented by the list pointer.
  The positions in the list are indexed starting with 0 (i.e. the first node in the
  list is at position n=0). If n >= nr_nodes, the new node is added to the end of
  the list. If n < 0, error.
*/
void ll_add_nth_node(linked_list_t* list, unsigned int n, const void* new_data);

/*
  * Removes the node at position n from the list
  whose pointer is sent as a parameter. The positions in the list
  are indexed from 0 (i.e. the first node in the list is at position n=0).
  If n >= nr_nodes - 1, the node at the end of the list is removed.
  If n < 0, error. The function returns a pointer to this newly removed node from the list.
*/
ll_node_t *ll_remove_nth_node(linked_list_t* list, unsigned int n);

/*
  * The function returns the number of nodes in the list
  whose pointer is sent as a parameter.
*/
unsigned int ll_get_size(linked_list_t* list);

/*
  * The procedure frees the memory used by all nodes in the list,
  and at the end, frees the memory used by the list structure and
  updates to NULL the value of the pointer to which the argument points
  (the argument is a pointer to a pointer).
*/
void ll_free(linked_list_t** pp_list);

#endif /* LINKEDLIST_H_ */
