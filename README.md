**Duca Andrei-Rares**
*311CA*

# Load_balancer:
The commands given in main() are executed, so that we have the following functionalities:

  * hash_function_servers(void *a) and hash_function_key(void *a): used to calculate the hash of a server replica or the hash of a key

  * find_first_higher(load_balancer *main, unsigned int hash): used to find the position of the replica which has a higher hash compared to the one given as a parameter. We can use it to search for the server on which we need to store a value , display it, and to find the server that will receive the elements from / give elements to my replica.

  * put_in_hashring(load_balancer *main, rep replica) and remove_from_hashring(load_balancer *main, unsigned int hash): are used to add the replica to the hashring and receive items from the clockwise neighbor server, or to remove the replica and distribute items to the neighbor server

  * and others like: init_load_balancer(), loader_add_server(load_balancer *main, int server_id), loader_remove_server(load_balancer *main, int server_id), loader_store(load_balancer *main, char *key, char *value, int *server_id),  loader_retrieve(load_balancer *main, char *key, int *server_id),  free_load_balancer(load_balancer *main) : which are used to initialize the load_balancer, add or remove a server, add a value to a server or display a value from a server, and finally to free memory.

# Server:
Help file to execute the commands given to a server from load_balancer, using the functions:

  * init_server_memory()
  * server_store(server_memory *server, char *key, char *value)
  * server_retrieve(server_memory *server, char *key)
  * server_remove(server_memory *server, char *key)
  * free_server_memory(server_memory *server)
  
which have the same attributes as the last functions in load_balancer.

# Hashtable:
Contains all the functions of working with a hashtable, necessary for the memory of each server in the load_balancer.

# Linkedlist:
Contains all functions for working with a linked list, necessary for each bucket within a hashtable in the memory of a server.

--------------------------------------------------------------------------------

# Other infos:
  	
  * The structure of the server contains a hashtable related to the storage of elements on a server.

  * The structure of the load_balancer contains the number of replicas stored in the hash_ring, the actual hash_ring of replicas of the servers and the vector of servers.

  * The structure of a replica contains the hash corresponding to the replica and the id of the server whose replica we have. It is structured in this way to always know, without other more complex operations, whose server is the replica on each position in the hash_ring.
