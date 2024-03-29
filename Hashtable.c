// Copyright 2022 Daraban Albert-Timotei
#include "./Hashtable.h"

int
compare_function_ints(void *a, void *b)
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

int
compare_function_strings(void *a, void *b)
{
	char *str_a = (char *)a;
	char *str_b = (char *)b;

	return strcmp(str_a, str_b);
}

/*
 * Hashing functions:
 */
unsigned int
hash_function_int(void *a)
{
	/*
	 * Credits: https://stackoverflow.com/a/12996028/7883884
	 */
	unsigned int uint_a = *((unsigned int *)a);

	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = (uint_a >> 16u) ^ uint_a;
	return uint_a;
}

unsigned int
hash_function_string(void *a)
{
	/*
	 * Credits: http://www.cse.yorku.ca/~oz/hash.html
	 */
	__uint8_t *puchar_a = (__uint8_t*) a;
	__uint64_t hash = 5381;
	int c;

	while ((c = *puchar_a++))
		hash = ((hash << 5u) + hash) + c; /* hash * 33 + c */

	return hash;
}

// Allocs memory for hashtable structure
hashtable_t *
ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
		int (*compare_function)(void*, void*), void (*free_val_func)(void *))
{
	hashtable_t *ht;
	ht = calloc(1, sizeof(hashtable_t));
	DIE(!ht, ALLOC_ERR);
	ht->buckets = calloc(hmax, sizeof(linked_list_t *));
	DIE(!ht->buckets, ALLOC_ERR);
	for (unsigned int i = 0; i < hmax; i++) {
		ht->buckets[i] = ll_create(sizeof(info_t));
	}
	ht->hmax = hmax;
	ht->hash_function = hash_function;
	ht->compare_function = compare_function;
	ht->free_val_func = free_val_func;
	return ht;
}

// Puts new entry in hashtable
void
ht_put(hashtable_t *ht, void *key, unsigned int key_size,
	void *value, unsigned int value_size, double load_factor)
{
	DIE(!ht, HT_U);

	resize_ht(ht, load_factor);

	unsigned int index = ht->hash_function(key) % ht->hmax;

	if (ht_has_key(ht, key)) {
		ht_remove_entry(ht, key);
	}

	info_t set;
	set.key = calloc(1, key_size);
	DIE(!set.key, ALLOC_ERR);
	memcpy(set.key, key, key_size);
	set.value = calloc(1, value_size);
	DIE(!set.value, ALLOC_ERR);
	memcpy(set.value, value, value_size);
	ll_add_nth_node(ht->buckets[index], ht->buckets[index]->size, &set);
	ht->size++;
}

// Return the value associated with a key
void *
ht_get(hashtable_t *ht, void *key)
{
	DIE(!ht, HT_U);

	unsigned int index = ht->hash_function(key) % ht->hmax;
	ll_node_t *node = ht->buckets[index]->head;

	while (node != NULL) {
		if(ht->compare_function(key, ((info_t *)node->data)->key) == 0)
			return ((info_t *)node->data)->value;
		node = node->next;
	}

	return NULL;
}

// Return 1 is a key is in hasttable, 0 otherwise
int
ht_has_key(hashtable_t *ht, void *key)
{
	DIE(!ht, HT_U);

	unsigned int index = ht->hash_function(key) % ht->hmax;

	ll_node_t *node = ht->buckets[index]->head;
	while (node)
	{
		if(ht->compare_function(key, ((info_t *)node->data)->key) == 0)
			return 1;
		node = node->next;
	}

	return 0;
}

// Removes entry from hashtable and free all data asociated with it
// Returns 1 is an entry was removed
int
ht_remove_entry(hashtable_t *ht, void *key)
{
	DIE(!ht, HT_U);

	unsigned int index = ht->hash_function(key) % ht->hmax;

	ll_node_t *node = ht->buckets[index]->head;
	unsigned int n = 0;
	while (node)
	{
		if(ht->compare_function(key, ((info_t *)node->data)->key) == 0) {
			ll_node_t *node = ll_remove_nth_node(ht->buckets[index], n);
			info_t *data = (info_t *)node->data;
			free(data->key);
			ht->free_val_func(data->value);
			free(data);
			free(node);
			ht->size--;
			return 1;
		}
		n++;
		node = node->next;
	}
	return 0;
}

// Frees memory of a hashtable
void
ht_free(hashtable_t *ht)
{
	DIE(!ht, HT_U);

	for (unsigned int i = 0; i < ht->hmax; i++) {
		ll_node_t *node = ht->buckets[i]->head;
		while(node) {
			ll_node_t *next = node->next;
			info_t *data = (info_t *)node->data;
			free(data->key);
			ht->free_val_func(data->value);
			free(data);
			free(node);
			node = next;
		}
		free(ht->buckets[i]);
	}

	free(ht->buckets);
	free(ht);
}

//  Resize array of linked-lists of an hashtable
void
resize_ht(hashtable_t *ht, double load_factor) {
	DIE(!ht, HT_U);

	if ((ht->size / ht->hmax) < load_factor)
		return;

	linked_list_t **new_buckets = calloc(ht->hmax * 2, sizeof(linked_list_t *));
	DIE(!new_buckets, ALLOC_ERR);
	unsigned int new_hmax = 2 * ht->hmax;

	for (unsigned int i = 0; i < new_hmax; i++)
		new_buckets[i] = ll_create(sizeof(info_t));

	for (unsigned int i = 0; i < ht->hmax; i++) {
		ll_node_t *node = ht->buckets[i]->head;
		while (node) {
			info_t *data = (info_t *)node->data;
			unsigned int index = ht->hash_function(data->key) % (new_hmax);
			ll_add_nth_node(new_buckets[index], new_buckets[index]->size, data);
			node = node->next;
		}
		ll_free(ht->buckets[i]);
	}
	free(ht->buckets);
	ht->hmax = new_hmax;
	ht->buckets = new_buckets;
}

// Return a sorted array of the hashtable entries
// It will compare using value and key
// It doesn't copy the data
info_t *
ht_sort(hashtable_t *ht, int (*compare_func)(const void *, const void *))
{
	if (!ht->size)
		return NULL;

	info_t *sorted_arr = calloc(ht->size, sizeof(info_t));
	DIE(!sorted_arr, ALLOC_ERR);
	unsigned int len = 0;

	for (unsigned int i = 0; i < ht->hmax; i++) {
		ll_node_t *node = ht->buckets[i]->head;
		while (node) {
			sorted_arr[len] = *(info_t *)(node->data);
			len++;
			node = node->next;
		}
	}

	qsort(sorted_arr, len, sizeof(info_t), compare_func);
	return sorted_arr;
}

// Prints hashtable key and values
void
ht_print(hashtable_t *ht, void (*print_data)(info_t *))
{
	if (!ht->size)
		return;

	for (unsigned int i = 0; i < ht->hmax; i++) {
		ll_node_t *node = ht->buckets[i]->head;
		while (node) {
			print_data(node->data);
			node = node->next;
		}
	}
}
