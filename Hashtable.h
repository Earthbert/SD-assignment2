// Copyright 2022 Daraban Albert-Timotei
#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./LinkedList.h"
#include "./utils.h"

#define HTMAX 10
#define LOAD_F 1.0

#define HT_U "Hashtable uninitialized"

typedef struct info_t info_t;
struct info_t
{
	void *key;
	void *value;
};


typedef struct hashtable_t hashtable_t;
struct hashtable_t
{
	// Arrays of linked lists in case of collisions
	linked_list_t **buckets;
	// Nr of elements in hashtable
	unsigned int size;
	// Size of the array
	unsigned int hmax;
	// Used at freeing values
	void (*free_val_func)(void *);
	unsigned int (*hash_function)(void*);
	int (*compare_function)(void*, void*);
};

// Hashtable functions

hashtable_t *
ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
		int (*compare_function)(void*, void*), void (*free_val_func)(void *));

void
ht_put(hashtable_t *ht, void *key, unsigned int key_size,
	void *value, unsigned int value_size, double load_factor);

void *
ht_get(hashtable_t *ht, void *key);

int
ht_has_key(hashtable_t *ht, void *key);

int
ht_remove_entry(hashtable_t *ht, void *key);

void
ht_free(hashtable_t *ht);

void
resize_ht(hashtable_t *ht, double load_factor);

info_t *
ht_sort(hashtable_t *ht, int (*compare_func)(const void *, const void *));

void
ht_print(hashtable_t *ht, void (*print_data)(info_t *));

// Compare key functions
int
compare_function_ints(void *a, void *b);

int
compare_function_strings(void *a, void *b);

// Hashing functions

unsigned int
hash_function_int(void *a);

unsigned int
hash_function_string(void *a);

#endif  // __HASHTABLE_H
