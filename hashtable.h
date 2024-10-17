/* Copyright 2023 Duca Andrei-Rares, 311CA */
#ifndef HASHTABLE_H_
#define HASHTABLE_H_

#include "linkedlist.h"

typedef struct info info;
struct info {
	void *key;
	void *value;
};

typedef struct hashtable_t hashtable_t;
struct hashtable_t {
	linked_list_t **buckets; /* Array of simply linked lists. */
	/* No. total of currently existing nodes in all buckets. */
	unsigned int size;
	unsigned int hmax; /* No. of buckets. */
	/* (Pointer to) Function to calculate the hash value associated
	with the keys. */
	unsigned int (*hash_function)(void*);
	/* (Pointer to) Function to compare two keys. */
	int (*compare_function)(void*, void*);
	/* (Pointer to) Function to free the memory occupied by key and value. */
	void (*key_val_free_function)(void*);
};

/*
 * Key comparison functions:
 */
int compare_function_ints(void *a, void *b);

int compare_function_strings(void *a, void *b);

/*
 * Hashing functions:
 */
unsigned int hash_function_servers(void *a);

unsigned int hash_function_key(void *a);

/*
  * Function called to release the memory occupied by the key
  and value of a hashtable pair.
  */
void key_val_free_function(void *data);

/*
  * Function called after allocating a hashtable to initialize it.
  Linked lists must also be allocated and initialized.
  */
hashtable_t *ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
		int (*compare_function)(void*, void*),
		void (*key_val_free_function)(void*));

void *ht_get(hashtable_t *ht, void *key);

/*
  * Attention! Although the key is sent as a void pointer (because its type
  is not imposed), when a new entry is created in the hashtable (in case the
  key is not already found in ht), a copy of the value must be created  to
  point key and the address of this copy must be saved in the info structure
  associated with the entry from ht. To know how many bytes must be allocated
  and copied, use the key_size parameter.
 
  * Motivation:
  * We need to copy the value to which key points because after a call
  put(ht, key_actual, value_actual), the value to which key_actual points
  can be altered (eg: *key_actual++). If we directly use the address of the
  key_actual pointer, the key of an entry in the hashtable would be changed
  from outside the hashtable. We don't want this, because there is a risk of
  ending up in a situation where we no longer know which key a certain value
  is registered to.
  */
void ht_put(hashtable_t *ht, void *key, unsigned int key_size,
	void *value, unsigned int value_size);

/*
  * The procedure that removes from the hashtable the entry associated with
  the key key. Careful! Care must be taken when releasing the entire memory
  used for an entry in the hashtable (that is, the memory for the copy of
  the key --see the observation from the put procedure--, for the info
  structure and for the Node structure from the chained list).
  */
void ht_remove_entry(hashtable_t *ht, void *key);

/*
  * The procedure that frees the memory used by all hashtable entries,
  after which it also frees the memory used to store the hashtable structure.
  */
void ht_free(hashtable_t *ht);

#endif  // HASHTABLE_H_
