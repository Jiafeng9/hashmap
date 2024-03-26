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


     // Allocate and initialize the mutex
    map->mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t)); // Allocate memory for the mutex
    if (!map->mutex || pthread_mutex_init(map->mutex, NULL) != 0) {
        // Mutex allocation or initialization failed, clean up and return NULL
        if (map->mutex) free(map->mutex); // Check if mutex was allocated before freeing
        free(map->table);
        free(map);
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
  if (!map) return INT_MAX;
  // TODO
  pthread_mutex_lock(map->mutex);

  //ts_entry_t temp =map->table[((unsigned int))%map->capacity];
  // Calculate the index for the given key using a hash function

  unsigned int index = (unsigned int)key % map->capacity;
  ts_entry_t *temp = map->table[index];

  while (temp != NULL) {
        if (temp->key == key) {
            // If the key is found, unlock the mutex and return the associated value
            int value = temp->value;
            pthread_mutex_unlock(map->mutex);
            return value;
        }
        temp = temp->next;
  }
 // Unlock the mutex if the key is not found
    pthread_mutex_unlock(map->mutex);
    return INT_MAX; // Return INT_MAX if the key is not found
}




/**
 * Associates a value associated with a given key.
 * @param map a pointer to the map
 * @param key a key
 * @param value a value
 * @return old associated value, or INT_MAX if the key was new
 */
int put(ts_hashmap_t *map, int key, int value) {
  if (!map) return INT_MAX; //None

  // TODO
  pthread_mutex_lock(map->mutex);
  
  unsigned int index = (unsigned int)key % map->capacity;
  ts_entry_t **temp = &(map->table[index]);// Use pointer to pointer for easy node addition

  while(*temp!=NULL){
    if ((*temp)->key ==key){
        int old_value =(*temp)->value;
        (*temp)->value =value;
        pthread_mutex_unlock(map->mutex);
        return old_value;

    }
    temp=&((*temp)->next);
  }

  // add new node 
    ts_entry_t *newNode = (ts_entry_t *)malloc(sizeof(ts_entry_t));
    if (newNode != NULL) { 
        newNode->key = key;
        newNode->value = value;
        newNode->next = NULL;
        *temp = newNode; // connet the new node 
    }

    pthread_mutex_unlock(map->mutex);
    return INT_MAX; // Return INT_MAX indicating that the key was new
}




/**
 * Removes an entry in the map
 * @param map a pointer to the map
 * @param key a key to search
 * @return the value associated with the given key, or INT_MAX if key not found
 */
int del(ts_hashmap_t *map, int key) {
  // TODO
  pthread_mutex_lock(map->mutex);
  
  unsigned int index = (unsigned int)key % map->capacity;
  ts_entry_t *temp = &(map->table[index]);// Use pointer to pointer for easy node addition


  while(temp->!=NULL){
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
    pthread_mutex_destroy(map->mutex);

    // Free the table of buckets
    free(map->table);

    // Free the hashmap structure itself
    free(map);
}