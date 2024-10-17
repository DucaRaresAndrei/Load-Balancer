/* Copyright 2023 Duca Andrei-Rares, 311CA */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"

int compare_function_ints(void *a, void *b)
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

int compare_function_strings(void *a, void *b)
{
	char *str_a = (char *)a;
	char *str_b = (char *)b;

	return strcmp(str_a, str_b);
}

void key_val_free_function(void *data)
{
	free(((info *)data)->value);
	free(((info *)data)->key);
}

hashtable_t *ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
		int (*compare_function)(void*, void*),
		void (*key_val_free_function)(void*))
{
	hashtable_t *ht = malloc(sizeof(struct hashtable_t));
	DIE(!ht, "hashtable malloc");

	ht->size = 0;
	ht->hmax = hmax;
	ht->hash_function = hash_function;
	ht->compare_function = compare_function;
	ht->key_val_free_function = key_val_free_function;

	ht->buckets = malloc(hmax * sizeof(linked_list_t *));
	DIE(!ht->buckets, "hashtable arraw of buckets malloc");

	for (unsigned int i = 0; i < hmax; i++) {
		ht->buckets[i] = ll_create(sizeof(info));
	}

	return ht;
}

void *ht_get(hashtable_t *ht, void *key)
{
	unsigned int my_key = ht->hash_function(key) % ht->hmax;
	linked_list_t *bucket = ht->buckets[my_key];

	ll_node_t *aux = bucket->head;

	for (unsigned int i = 0; i < bucket->size; i++) {
		if (!ht->compare_function(key, ((info *)aux->data)->key)) {
			return ((info *)aux->data)->value;
		}
		aux = aux->next;
	}

	return NULL;
}

void ht_put(hashtable_t *ht, void *key, unsigned int key_size,
	void *value, unsigned int value_size)
{
	info data;

	unsigned int my_key = ht->hash_function(key) % ht->hmax;
	linked_list_t *bucket = ht->buckets[my_key];

	ll_node_t *aux = bucket->head;

	for (unsigned int i = 0; i < bucket->size; i++) {
		if (!ht->compare_function(key, ((info *)aux->data)->key)) {
			break;
		}
		aux = aux->next;
	}

	if (!aux) {
		data.key = malloc(key_size);
		memcpy(data.key, key, key_size);

		data.value = malloc(value_size);
		memcpy(data.value, value, value_size);

		ll_add_nth_node(bucket, 0, &data);
		ht->size++;
	} else {
		memcpy(((info *)aux->data)->value, value, value_size);
	}
}

void ht_remove_entry(hashtable_t *ht, void *key)
{
	unsigned int my_key = ht->hash_function(key) % ht->hmax;
	linked_list_t *bucket = ht->buckets[my_key];

	ll_node_t *aux = bucket->head;

	for (unsigned int i = 0; i < bucket->size; i++) {
		if (!ht->compare_function(key, ((info *)aux->data)->key)) {
			ll_node_t *remove = ll_remove_nth_node(bucket, i);
			key_val_free_function(remove->data);

			free(remove->data);
			free(remove);

			ht->size--;
			break;
		}
		aux = aux->next;
	}
}

void ht_free(hashtable_t *ht)
{
    ll_node_t *aux;
	for (unsigned int i = 0; i < ht->hmax; i++) {
		if (ht->buckets[i]->head != NULL) {
			aux = ht->buckets[i]->head;

			while (aux != NULL) {
				key_val_free_function(aux->data);
				aux = aux->next;
			}
		}
		ll_free(&ht->buckets[i]);
	}

	free(ht->buckets);
	free(ht);
}
