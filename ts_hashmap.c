#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ts_hashmap.h"

/**
 * Creates a new thread-safe hashmap. 
 *
 * @param capacity initial capacity of the hashmap.
 * @return a pointer to a new thread-safe hashmap.
 */
ts_hashmap_t *initmap(int capacity) {
  // Allocate memory for the hashmap
    ts_hashmap_t *map = (ts_hashmap_t *)malloc(sizeof(ts_hashmap_t));
    if (!map) return NULL; // Check if memory allocation failed

    // Allocate memory for the table with the given capacity
    map->table = (ts_entry_t **)malloc(sizeof(ts_entry_t *) * capacity);
    if (!map->table) { // Check if memory allocation for table failed
        free(map); // Free the allocated memory for the hashmap
        return NULL;
    }

    // Initialize the table pointers to NULL
    for (int i = 0; i < capacity; ++i) {
        map->table[i] = NULL;
    }


    // Initialize the mutex
    if (pthread_mutex_init(map->mutex, NULL) != 0) {
      // Mutex initialization failed, clean up and return NULL
        free(map->table); // Free the table
        free(map);        // Free the hashmap
        return NULL;
    }

    // Initialize the rest of the fields
    map->capacity = capacity;
    map->size = 0;     // number of entries currently stored
    map->numOps = 0;   // the number of operations (put, get, del) performed

    return map;
}


/**
 * Obtains the value associated with the given key.
 * @param map a pointer to the map
 * @param key a key to search
 * @return the value associated with the given key, or INT_MAX if key not found
 */
int get(ts_hashmap_t *map, int key) {
  // TODO
  pthread_mutex_init(map->mutex, NULL);
  ts_entry_t temp =map->table[((unsigned int))%map->capacity];
  while(!temp->next !=NULL){
    if (temp->key ==key){
      return key;
    }
    temp=temp->next;
  }
  pthread_mutex_destroy(map->mutex);
  return INT_MAX;
}

/**
 * Associates a value associated with a given key.
 * @param map a pointer to the map
 * @param key a key
 * @param value a value
 * @return old associated value, or INT_MAX if the key was new
 */
int put(ts_hashmap_t *map, int key, int value) {
  // TODO
  pthread_mutex_init(map->mutex, NULL);
  ts_entry_t temp =map->table[((unsigned int))%map->capacity];
  while(!temp->next !=NULL){
    if (temp->key ==key){
        temp->value =value;
    }
    temp=temp->next;
  }
  temp->key=key;
  temp->value=value;
  pthread_mutex_destroy(map->mutex);
  return INT_MAX;
}

/**
 * Removes an entry in the map
 * @param map a pointer to the map
 * @param key a key to search
 * @return the value associated with the given key, or INT_MAX if key not found
 */
int del(ts_hashmap_t *map, int key) {
  // TODO
  pthread_mutex_init(map->mutex, NULL);
  ts_entry_t temp =map->table[((unsigned int))%map->capacity];
  while(!temp->next !=NULL){
    if (temp->key ==key){
        temp->key ==temp->next->key
        temp->value ==temp->nest->value
        temp=temp->next  // need lock?
    }
  }
  return INT_MAX;
}


/**
 * Prints the contents of the map (given)
 */
void printmap(ts_hashmap_t *map) {
  for (int i = 0; i < map->capacity; i++) {
    printf("[%d] -> ", i);
    ts_entry_t *entry = map->table[i];
    while (entry != NULL) {
      printf("(%d,%d)", entry->key, entry->value);
      if (entry->next != NULL)
        printf(" -> ");
      entry = entry->next;
    }
    printf("\n");
  }
}

/**
 * Free up the space allocated for hashmap
 * @param map a pointer to the map
 */
void freeMap(ts_hashmap_t *map) {
  // TODO: iterate through each list, free up all nodes
  // TODO: free the hash table
  // TODO: destroy locks
   // Iterate through each bucket
    for (int i = 0; i < map->capacity; i++) {
        // Free all nodes in the linked list at table[i]
        ts_entry_t *current_entry = map->table[i];
        while (current_entry != NULL) {
            ts_entry_t *temp = current_entry;
            current_entry = current_entry->next;
            free(temp); // Free the node
        }
    }
    // Destroy the mutex
    pthread_mutex_destroy(&map->mutex);

    // Free the table of buckets
    free(map->table);

    // Free the hashmap structure itself
    free(map);
}