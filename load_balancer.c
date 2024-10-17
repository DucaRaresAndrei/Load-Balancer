/* Copyright 2023 Duca Andrei-Rares, 311CA */
#include <stdlib.h>
#include <string.h>

#include "load_balancer.h"

/*
 * Hashing functions:
 */
unsigned int hash_function_servers(void *a)
{
    unsigned int uint_a = *((unsigned int *)a);

    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = (uint_a >> 16u) ^ uint_a;
    return uint_a;
}

unsigned int hash_function_key(void *a)
{
    unsigned char *puchar_a = (unsigned char *)a;
    unsigned int hash = 5381;
    int c;

    while ((c = *puchar_a++))
        hash = ((hash << 5u) + hash) + c;

    return hash;
}

/*
 *The function returns the position of the first replica in the
 hash_ring that has a hash greater than the hash of my replica
 */
int find_first_higher(load_balancer *main, unsigned int hash)
{
    for (int i = 0; i < main->size; i++) {
        rep server = main->hash_ring[i];
        
        if (compare_function_ints(&hash, &server.hash) < 0) {
            return i;
        }
    }

    return main->size;
}

/*
 *The function enters the 3 replicas of the server in the hash_ring and
 takes from the neighboring servers to the right, clockwise, the keys and
 their associated values that have a lower hash than the hash of my replica.

 If my replica will be inserted at position 0, the key-value pairs higher
 than the hash of the replica at the last position will be taken
 */
void put_in_hashring(load_balancer *main, rep replica)
{
    int new_poz = find_first_higher(main, replica.hash);
    main->size++;

    rep *aux_hash_ring = realloc(main->hash_ring, main->size * sizeof(rep));
    DIE(!aux_hash_ring, "realloc of hash_ring");
    main->hash_ring = aux_hash_ring;

    for (int i = main->size - 1; i > new_poz; i--) {
        aux_hash_ring[i] = aux_hash_ring[i - 1];
    }
    aux_hash_ring[new_poz] = replica;

    int old_poz, prev_poz;
    if (new_poz == main->size - 1) {
        old_poz = 0;
        prev_poz = new_poz - 1;
    } else {
        old_poz = new_poz + 1;
        if (new_poz == 0)
            prev_poz = main->size - 1;
        else
            prev_poz = new_poz - 1;
    }

    if (aux_hash_ring[old_poz].server_id == aux_hash_ring[new_poz].server_id)
        return;

    int server_id = aux_hash_ring[old_poz].server_id;
    server_memory *old_server = main->sv[server_id];
    hashtable_t *ht = old_server->ht;

    unsigned int hash_prev = aux_hash_ring[prev_poz].hash;

    server_id = aux_hash_ring[new_poz].server_id;
    server_memory *new_server = main->sv[server_id];

    for (int i = 0; i < HMAX; i++) {
        linked_list_t *bucket = ht->buckets[i];
        ll_node_t *aux = bucket->head;

        for (unsigned int j = 0; j < bucket->size; j++) {
            char *object_key = ((info *)aux->data)->key;
            unsigned int hash_key = hash_function_key(object_key);

            if (new_poz == 0) {
                if (!((compare_function_ints(&hash_key, &hash_prev) > 0) ||
                    (compare_function_ints(&hash_key, &replica.hash) <= 0))) {
                    aux = aux->next;
                    continue;
                }
            } else {
                if (!((compare_function_ints(&hash_key, &hash_prev) > 0) &&
                    (compare_function_ints(&hash_key, &replica.hash) <= 0))) {
                    aux = aux->next;
                    continue;
                }
            }

            server_store(new_server, object_key, ((info *)aux->data)->value);

            aux = aux->next;
            j--;

            server_remove(old_server, object_key);
        }
    }
}

/*
 * The function removes from the hash_ring the 3 replicas of the server,
 mapping its elements to the servers that have the replica with the position
 immediately higher than the position on which my replica is located
 in the hash_ring
 */
void remove_from_hashring(load_balancer *main, unsigned int hash)
{
    int old_poz = find_first_higher(main, hash);
    old_poz--;

    int server_id = main->hash_ring[old_poz].server_id;

    int new_poz, prev_poz;
    if (old_poz == 0) {
        prev_poz = main->size - 1;
        new_poz = old_poz + 1;
    } else if (old_poz == main->size - 1) {
        prev_poz = old_poz - 1;
        new_poz = 0;
    } else {
        prev_poz = old_poz - 1;
        new_poz = old_poz + 1;
    }

    unsigned int hash_prev = main->hash_ring[prev_poz].hash;
    unsigned int hash_new = main->hash_ring[new_poz].hash;

    server_memory *old_server = main->sv[server_id];
    hashtable_t *ht = old_server->ht;

    int new_sv_id = main->hash_ring[new_poz].server_id;
    server_memory *new_server = main->sv[new_sv_id];

    for (int i = old_poz; i < main->size - 1; i++)
        main->hash_ring[i] = main->hash_ring[i + 1];

    main->size--;
    rep *aux_hash_ring = realloc(main->hash_ring, main->size * sizeof(rep));
    DIE(!aux_hash_ring, "realloc of hash_ring");
    main->hash_ring = aux_hash_ring;

    if (server_id == new_sv_id)
        return;

    for (int i = 0; i < HMAX; i++) {
        linked_list_t *bucket = ht->buckets[i];
        ll_node_t *aux = bucket->head;

        for (unsigned int j = 0; j < bucket->size; j++) {
            char *object_key = ((info *)aux->data)->key;
            unsigned int hash_key = hash_function_key(object_key);

            if (old_poz == 0) {
                if (!((compare_function_ints(&hash_key, &hash_prev) > 0) ||
                    (compare_function_ints(&hash_key, &hash_new) <= 0))) {
                    aux = aux->next;
                    continue;
                }
            } else if (old_poz == main->size) {
                if (!(compare_function_ints(&hash_key, &hash_prev) > 0)) {
                    aux = aux->next;
                    continue;
                }
            } else {
                if (!((compare_function_ints(&hash_key, &hash_prev) > 0) &&
                    (compare_function_ints(&hash_key, &hash_new) <= 0))) {
                    aux = aux->next;
                    continue;
                }
            }

            server_store(new_server, object_key, ((info *)aux->data)->value);

            aux = aux->next;
            j--;

            server_remove(old_server, object_key);
        }
    }
}

/*
 * The function initializes the load_balancer and its list of servers
 */
load_balancer *init_load_balancer()
{
    load_balancer *main_sv = malloc(sizeof(load_balancer));
    DIE(!main_sv, "load_balancer malloc");

    main_sv->hash_ring = NULL;

    main_sv->sv = calloc(MAX_SERVERS, sizeof(server_memory *));
    DIE(!main_sv->sv, "array of servers alloc");

    main_sv->size = 0;

    return main_sv;
}

/*
 *The function initiates the server addressed to server_id and enters 
 it in the list of servers.
 */
void loader_add_server(load_balancer *main, int server_id)
{
    main->sv[server_id] = init_server_memory();
    unsigned int label = server_id;
    unsigned int hash_rep = hash_function_servers(&label);

    rep replica1, replica2, replica3;

    replica1.hash = hash_rep;
    replica1.server_id = server_id;
    put_in_hashring(main, replica1);

    label = 100000 + server_id;
    hash_rep = hash_function_servers(&label);

    replica2.hash = hash_rep;
    replica2.server_id = server_id;
    put_in_hashring(main, replica2);

    label = 200000 + server_id;
    hash_rep = hash_function_servers(&label);

    replica3.hash = hash_rep;
    replica3.server_id = server_id;
    put_in_hashring(main, replica3);
}

/*
 * The function deletes the server addressed to server_id from load_balancer
 */
void loader_remove_server(load_balancer *main, int server_id)
{
    unsigned int label = server_id;
    unsigned int hash_rep = hash_function_servers(&label);
    remove_from_hashring(main, hash_rep);

    label = 100000 + server_id;
    hash_rep = hash_function_servers(&label);
    remove_from_hashring(main, hash_rep);

    label = 200000 + server_id;
    hash_rep = hash_function_servers(&label);
    remove_from_hashring(main, hash_rep);

    free_server_memory(main->sv[server_id]);
    main->sv[server_id] = NULL;
}

/*
 * The function stores the key-value pair on the related server
 */
void loader_store(load_balancer *main, char *key, char *value, int *server_id)
{
    unsigned int hash_key = hash_function_key(key);
    int poz = find_first_higher(main, hash_key);

    if (poz == main->size)
        poz = 0;

    *server_id = ((rep)main->hash_ring[poz]).server_id;
    server_store(main->sv[*server_id], key, value);
}

/*
 * The function returns the value of the associated key, as well as
 the id of the server on which the value is stored
 */
char *loader_retrieve(load_balancer *main, char *key, int *server_id)
{
    unsigned int hash_key = hash_function_key(key);
    int poz = find_first_higher(main, hash_key);

    if (poz == main->size)
        poz = 0;

    *server_id = ((rep)main->hash_ring[poz]).server_id;
    return server_retrieve(main->sv[*server_id], key);
}

/*
 * The function releases the memory of each existing server,
 of the hash_ring and of the load_balancer
 */
void free_load_balancer(load_balancer *main)
{
    for (int i = 0; i < MAX_SERVERS; i++) {
        if (!main->size)
            break;

        if (main->sv[i]) {
            main->size -= 3;
            free_server_memory(main->sv[i]);
        }
    }

    free(main->sv);
    free(main->hash_ring);
    free(main);
}
