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
ts_hashmap_t *initmap(int capacity)
{
    // Allocate memory for the hashmap
    ts_hashmap_t *map = (ts_hashmap_t *)malloc(sizeof(ts_hashmap_t));
    if (!map)
        return NULL; 

    // Allocate memory for the table 
    map->table = (ts_entry_t **)malloc(sizeof(ts_entry_t *) * capacity);
    if (!map->table)
    {              // Check if memory allocation for table failed
        free(map); // Free the allocated memory for the hashmap
        return NULL;
    }
    // Initialize the table pointers to NULL
    for (int i = 0; i < capacity; ++i)
    {
        map->table[i] = NULL;
    }

    // Allocate and initialize the mutex
    
    map->mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t)*capacity); // Allocate memory for the mutex
    // Initialize each bucket mutex
    for (int i = 0; i < capacity; ++i)
    {
        if (pthread_mutex_init(&map->mutex[i], NULL) != 0)
        {
            // Cleanup on failure
            while (--i >= 0)
            pthread_mutex_destroy(&map->mutex[i]);
            free(map->mutex);
            free(map->table);
            free(map);
            return NULL;
        }
    }

    if (pthread_mutex_init(&map->opsMutex, NULL) != 0)
    {
        free(map->mutex);
        free(map->table);
        free(map);
        return NULL;
    }

    if (pthread_mutex_init(&map->sizeMutex, NULL) != 0)
    {
        free(map->mutex);
        free(map->table);
        free(map);
        return NULL;
    }

    // Initialize the rest of the fields
    map->capacity = capacity;
    map->size = 0;   // number of entries currently stored
    map->numOps = 0; // the number of operations (put, get, del) performed
    return map;
}

/**
 * Obtains the value associated with the given key.
 * @param map a pointer to the map
 * @param key a key to search
 * @return the value associated with the given key, or INT_MAX if key not found
 */
int get(ts_hashmap_t *map, int key)
{
    if (!map)
        return INT_MAX;

    unsigned int index = (unsigned int)key % map->capacity;

    pthread_mutex_lock(&map->mutex[index]);

    ts_entry_t *temp = map->table[index];

    while (temp != NULL)
    {
        if (temp->key == key)
        {
            // If the key is found, unlock the mutex and return the associated value
            int value = temp->value;
            pthread_mutex_unlock(&map->mutex[index]);

            // Safe update of numOps, if needed
            safe_update_numOps(map);
            return value;
        }
        temp = temp->next;
        
    }
    // Unlock the mutex if the key is not found
    pthread_mutex_unlock(&map->mutex[index]);
    // Safe update of numOps, even if key not found
    safe_update_numOps(map);
    return INT_MAX; // Return INT_MAX if the key is not found
}



/**
 * Associates a value associated with a given key.
 * @param map a pointer to the map
 * @param key a key
 * @param value a value
 * @return old associated value, or INT_MAX if the key was new
 */
int put(ts_hashmap_t *map, int key, int value)
{
    if (!map)
        return INT_MAX; // Ensure map is not NULL


    unsigned int index = (unsigned int)key % map->capacity;

    pthread_mutex_lock(&map->mutex[index]);

    ts_entry_t **temp = &map->table[index]; 
    while (*temp != NULL)
    {
        if ((*temp)->key == key)
        {
            // If the key exists, update its value and return the old value
            int old_value = (*temp)->value;
            (*temp)->value = value;
            pthread_mutex_unlock(&map->mutex[index]); // Unlock before returning
            safe_update_size(map,1);
            safe_update_numOps(map);
            return old_value;
        }
        temp = &((*temp)->next); // Move to next node
    }

    // Add new node at the correct position (in front of current *temp)
    ts_entry_t *newNode = (ts_entry_t *)malloc(sizeof(ts_entry_t));
    if (newNode != NULL)
    {
        newNode->key = key;
        newNode->value = value;
        newNode->next = *temp; // The current *temp is NULL here, so newNode->next should also be NULL
        *temp = newNode;       // Connect the new node

        // Successfully added a new node, so unlock and update size and numOps
        pthread_mutex_unlock(&map->mutex[index]);

        // safely increment the size
        safe_update_size(map,1);
        // safely increment numOps
        safe_update_numOps(map);
        return INT_MAX; 
    }

    // If newNode is NULL, malloc failed, but we still need to unlock
    pthread_mutex_unlock(&map->mutex[index]);
    //safe_update_numOps(map);
    return INT_MAX; 

}

/**
 * Removes an entry in the map
 * @param map a pointer to the map
 * @param key a key to search
 * @return the value associated with the given key, or INT_MAX if key not found
 */
int del(ts_hashmap_t *map, int key)
{
    if (!map)
        return INT_MAX;

    unsigned int index = (unsigned int)key % map->capacity;

    pthread_mutex_lock(&map->mutex[index]);

    ts_entry_t **temp = &map->table[index];
    while (*temp != NULL)
    {
        if ((*temp)->key == key)
        {
            ts_entry_t *toDelete = *temp;
            int value = toDelete->value;


            *temp = toDelete->next;

            // Free the deleted node
            free(toDelete);

            // Unlock the mutex after the deletion
            pthread_mutex_unlock(&map->mutex[index]);

            // Update size and numOps safely (assuming you have functions/methods for that)
            // safe_decrement_size(map);
            safe_update_numOps(map);
            safe_update_size(map,-1);

            return value; // Return the deleted node's value
        }
        temp = &(*temp)->next;
    }

    // Unlock the mutex if the key is not found
    pthread_mutex_unlock(&map->mutex[index]);

    // Even if not found, we consider it an operation
    safe_update_numOps(map);
    safe_update_size(map,-1);
    return INT_MAX; // Key not found, return INT_MAX
}

/**
 * Prints the contents of the map (given)
 */
void printmap(ts_hashmap_t *map)
{
    for (int i = 0; i < map->capacity; i++)
    {
        printf("[%d] -> ", i);
        ts_entry_t *entry = map->table[i];
        while (entry != NULL)
        {
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
// TODO: iterate through each list, free up all nodes
// TODO: free the hash table
// TODO: destroy locks
void freeMap(ts_hashmap_t *map)
{
    if (!map)
        return; // Ensure map is not NULL

    // Iterate through each bucket
    for (int i = 0; i < map->capacity; i++)
    {
        // Free all nodes in the linked list at table[i]
        ts_entry_t *current_entry = map->table[i];
        while (current_entry != NULL)
        {
            ts_entry_t *temp = current_entry;
            current_entry = current_entry->next;
            free(temp); // Free the node
        }

        // Destroy the mutex for this bucket
        pthread_mutex_destroy(&map->mutex[i]);
    }

    // Free the array of mutexes
    free(map->mutex);

    // Free the table of buckets
    free(map->table);

    // If there's a separate mutex for numOps, destroy it here
    // pthread_mutex_destroy(&map->opsMutex);

    // Free the hashmap structure itself
    free(map);
}

void safe_update_numOps(ts_hashmap_t *map)
{
    pthread_mutex_lock(&map->opsMutex);
    map->numOps++;
    pthread_mutex_unlock(&map->opsMutex);
}

void safe_update_size(ts_hashmap_t *map,int nu)
{
    pthread_mutex_lock(&map->sizeMutex);
    map->size=map->size +nu;
    pthread_mutex_unlock(&map->sizeMutex);
}