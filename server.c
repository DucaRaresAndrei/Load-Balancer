/* Copyright 2023 Duca Andrei-Rares, 311CA */
#include "server.h"

/*
 * The function initializes the memory of a server, as well as its hashtable
 */
server_memory *init_server_memory()
{
	server_memory *server = malloc(sizeof(server_memory));
	DIE(!server, "server alloc");

	server->ht = ht_create(HMAX, hash_function_key,
				compare_function_strings, key_val_free_function);

	return server;
}

/*
 * The function puts the key and its associated value in the server's hashtable
 */
void server_store(server_memory *server, char *key, char *value)
{
	ht_put(server->ht, key, strlen(key) + 1, value, strlen(value) + 1);
}

/*
 * The function returns the value associated with the key
 */
char *server_retrieve(server_memory *server, char *key)
{
	return ht_get(server->ht, key);
}

/*
 * The function deletes the key and its associated value from the server
 */
void server_remove(server_memory *server, char *key)
{
	ht_remove_entry(server->ht, key);
}

/*
 * The function deletes all existing elements on the server, 
 and then frees its memory
 */
void free_server_memory(server_memory *server)
{
	ht_free(server->ht);
	free(server);
}
