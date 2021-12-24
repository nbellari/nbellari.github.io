---
layout: post
title: "Implementing a Hash Table"
date:  2021-12-24 18:59:28 +0530
categories: programming
---

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

## Implementation

We represent a hash table as below:

```c
/* A hash table is one that stores the key-value pairs */
typedef struct {
    long  ht_n_buckets;
    long  ht_count;
    ht_entry_t **ht_buckets; 
} htable_t;
```

This holds all the buckets together and keeps a tab on them. A bucket entry looks like this:

```c
/* A hash table entry is a collection of key and value */
typedef struct ht_entry {
    char *key;
    char *value;
    struct ht_entry *next, *prev; /* Doubly linked list for ease of addition/removal */
} ht_entry_t;
```

We will visit the `next` and `prev` later. For now, they are not used. A hash table is created as follows:

```c
void *
hash_table_create(long n_buckets)
{
    htable_t *table;

    n_buckets = n_buckets ? next_prime(n_buckets) :
                     next_prime(HT_DEFAULT_BUCKET_SIZE);

    table = calloc(1, sizeof(htable_t));
    if (table == NULL) {
        /* Memory allocation failure - serious */
        return NULL;
    }

    table->ht_buckets = calloc(n_buckets, sizeof(ht_entry_t *));
    if (table->ht_buckets == NULL) {
        /* Memory allocation failure - serious */
        free(table);
        return NULL;
    }

    table->ht_n_buckets = n_buckets;

    return table;
}
```

The client can specify the number of buckets at the time of creation, but it if it's skipped, then a default is used. It is important to keep the bucket count a prime number as it will result in lesser number of collisions. We allocate both the table as well as the bucket pointers which points a `ht_entry_t`. `next_prime` is a simple routine:

```c
bool
is_prime(long x)
{
    long sqrt_x;

    if (x < 2) return false;
    if (x < 4) return true;
    if (x%2 == 0) return false;

    sqrt_x = floor(sqrt((double)x));
    for (long i=3; i<sqrt_x; i++) {
        if (x%i == 0) return false;
    }

    return true;
}

long
next_prime(long x)
{
    while (!is_prime(x)) {
        x++;
    }

    return x;
}
```

### Hashing methods

Before we delve into other APIs, lets look at how hashing is done and how hash collisions are resolved (in this implementation of course!). When we hash a key and it points to a bucket, it can be the case that there can be an entry already present in it. In that case, we have a hash collision. There are many ways to resolve a hash collision like:

* chaining the keys pointing to the same hash together
* doing a linear probing - start from the current collision entry and continue until we find an empty bucket
* use another hash function to find the next bucket etc.

The trouble with the first two methods are - the cost of locating a key in the hash table increases when the chain gets bigger or when the entries to be searched linearly which defeats the purpose of a hash table. The third one is reasonably close because we still use hash functions to locate an empty bucket. So, then, to get a bucket, we use the following logic:

```c
static long
hash_table_get_bucket(htable_t *table, char *key, uint8_t new)
{
    long     i, idx;
    long     hash1_idx, hash2_idx;
    
    hash1_idx = hash1_bucket(key, table->ht_n_buckets);
    hash2_idx = hash2_bucket(key, table->ht_n_buckets);

    for (i=0; i<table->ht_n_buckets; i++) {
        /* (h1 + i*h2) % buckets */
        idx = (hash1_idx + i * hash2_idx) % table->ht_n_buckets;
        
        /* If the slot is empty, the loop terminates */
        if (!table->ht_buckets[idx]) {
            if (new)
                return idx;
            else
                return -1;
        } else if (strcmp(table->ht_buckets[idx]->key, key) == 0) {
            if (!new) {
                return idx;
            }
        }
        /* continue, otherwise */
    }

    /* If we reach here, it means the string did not get mapped to
     * any bucket. Not sure, if this is a valid case
     */
    assert(0);
}
```

As we can see, we take the bucket to be a weighted sum of two hash indexes. In the first run, we don't consider the second hash at all (i is 0). If we find an empty slot, then good! Otherwise we continue the iteration (now considering the second hash also into account) until we find an empty slot. Because it is possible that `idx` can circle back to the same index (there is modulo, you see, it is necessary to keep a tab on the number of iteartions.

Adding an element to a hash table, is simple, given this:

```c
void
hash_table_add(void *hash_table, char *key, char *value)
{
    htable_t    *table = (htable_t *)hash_table;
    ht_entry_t  *entry = hash_table_entry_get(table, key);

    if (entry) {
        /* Entry already present, overwrite the value */
        free(entry->value);
        entry->value = strdup(value);
    } else {
        /* Entry does not exist, add it */
        hash_table_insert(hash_table, key, value);
    }
}

static void
hash_table_insert(void *hash_table, char *key, char *value)
{
    long            idx;
    ht_entry_t      *entry = NULL;
    htable_t        *table = (htable_t *)hash_table;

    /* This does not return without a valid index */
    idx = hash_table_get_bucket(hash_table, key, 1/*new*/);
    entry = ht_entry_create(key, value);

    table->ht_buckets[idx] = entry;
}
```

Deleting is similar:

```c
void
hash_table_delete(void *hash_table, char *key)
{
    htable_t    *table = (htable_t *)hash_table;
    long        idx = hash_table_get_bucket(table, key, 0/*new*/);
    ht_entry_t  *entry;

    if (idx < 0) {
        /* nothing to do */
        return;
    }

    entry = table->ht_buckets[idx];
    table->ht_buckets[idx] = NULL;
    ht_entry_delete(entry);
    return;
}
```

We call the same `hash_table_get_bucket` but with a `0` so instead of returning a valid index, it returns `-1` highlighting that such an element does not exist in the hash table. Needless to say, a hash table search is pretty straight forward:

```c
char *
hash_table_search(void *hash_table, char *key)
{
    htable_t *table = (htable_t *)hash_table;
    ht_entry_t  *entry = hash_table_entry_get(table, key);

    if (entry) {
        return entry->value;
    }

    return NULL;
}
```

If we look at the API of the hash table, we see that there is also a `hash_table_destroy` provided. It is possible that there are some elements in the hash table, so we need to walk through the table and clean up the elements before destroying the hash table:

```c
void
hash_table_destroy(void *hash_table)
{
    htable_t    *table = (htable_t *)hash_table;

    if (!table) {
        return;
    }

    /* Loop through all the elements, free all the elements
     * and then destroy the hash table
     */
    ht_entry_flush_all(table);

    /* Free the buckets */
    free(table->ht_buckets);

    /* Free the table */
    free(table);

    return;
}

static void
ht_entry_flush_all(htable_t *table)
{
    int         i;
    ht_entry_t  *entry, *next;

    for (i=0; i<table->ht_n_buckets; i++) {
        if (table->ht_buckets[i]) {
            entry = table->ht_buckets[i];
            /* Delete all the entries in the bucket */
            while (entry) {
                next = entry->next;
                ht_entry_delete(entry);
                entry = next;               
            }
        }
    }

}
```

The function `ht_entry_flush_all` takes care of flushing the elements properly. The `prev` and the `next` fields are not used here, but they are still referenced here in the function. This is just a placeholder for a future modification. In case we decide to change the hash collision resolution method and go for chaining, then this will be handy.
