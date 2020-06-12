# Writing a Hash Table

This is my journey of implementing a hash table. There are two main things when we are implementing a data structure - providing the necessary interface for the clients to use and the actual implementation details.

## Interface

Here is a take at the interface:

```c
#ifndef _HASH_TABLE_H_
#define _HASH_TABLE_H_

/* Functions defined by the hash table library for clients */

/* Function to create a hash table */
void *hash_table_create(long num_entries);

/* Function to search for the presence of a key */
char *hash_table_search(void *hash_table, char *key);

/* Function to add an entry to the hash table */
void hash_table_add(void *hash_table, char *key, char *value);

/* Function to delete an entry from the hash table */
void hash_table_delete(void *hash_table, char *key);

/* Function to destroy the hash table */
void hash_table_destroy(void *hash_table);

#endif
```

As we can see above, this is a hash table interface that maps strings to any value (basically a pointer). We will tal about later on how to support a key of any type. The API is pretty self-explanatory. 
