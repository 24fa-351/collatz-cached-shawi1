#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cache.h"
#include "collatz.h"
#include "config.h"

Cache *create_cache(int capacity, const char *policy)
{
    Cache *cache = (Cache *)malloc(sizeof(Cache));
    cache->capacity = capacity;
    cache->size = 0;
    cache->hits = 0;
    cache->misses = 0;
    cache->head = NULL;
    cache->tail = NULL;

    if (strcmp(policy, "LRU") == 0)
    {
        cache->policy = CACHE_LRU;
    }
    else if (strcmp(policy, "FIFO") == 0)
    {
        cache->policy = CACHE_FIFO;
    }

    return cache;
}

CacheEntry *find_cache_entry(Cache *cache, int number)
{
    CacheEntry *current = cache->head;
    while (current != NULL)
    {
        if (current->number == number)
        {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Move a cache entry to the head (for LRU policy)
void move_to_head(Cache *cache, CacheEntry *entry)
{
    if (entry == cache->head)
        return;

    // Remove entry from current position
    if (entry->prev != NULL)
    {
        entry->prev->next = entry->next;
    }
    if (entry->next != NULL)
    {
        entry->next->prev = entry->prev;
    }
    if (entry == cache->tail)
    {
        cache->tail = entry->prev;
    }

    // Move to head
    entry->prev = NULL;
    entry->next = cache->head;
    if (cache->head != NULL)
    {
        cache->head->prev = entry;
    }
    cache->head = entry;
    if (cache->tail == NULL)
    {
        cache->tail = entry;
    }
}

// Add a new entry to the cache (for LRU and FIFO)
void add_to_cache(Cache *cache, int number, int steps)
{
    if (cache->size == cache->capacity)
    {
        // Evict an entry based on the cache policy
        CacheEntry *to_evict;
        if (cache->policy == CACHE_FIFO)
        {
            // FIFO: Evict the oldest (head)
            to_evict = cache->head;
            cache->head = to_evict->next;
            if (cache->head != NULL)
            {
                cache->head->prev = NULL;
            }
            else
            {
                cache->tail = NULL; // Cache is now empty
            }
        }
        else if (cache->policy == CACHE_LRU)
        {
            // LRU: Evict the least recently used (tail)
            to_evict = cache->tail;
            cache->tail = to_evict->prev;
            if (cache->tail != NULL)
            {
                cache->tail->next = NULL;
            }
            else
            {
                cache->head = NULL; // Cache is now empty
            }
        }
        free(to_evict);
        cache->size--;
    }

    // Create a new cache entry
    CacheEntry *new_entry = (CacheEntry *)malloc(sizeof(CacheEntry));
    new_entry->number = number;
    new_entry->steps = steps;
    new_entry->next = NULL;
    new_entry->prev = NULL;

    if (cache->policy == CACHE_LRU)
    {
        // LRU: Add to the head
        new_entry->next = cache->head;
        if (cache->head != NULL)
        {
            cache->head->prev = new_entry;
        }
        cache->head = new_entry;
        if (cache->tail == NULL)
        {
            cache->tail = new_entry;
        }
    }
    else if (cache->policy == CACHE_FIFO)
    {
        // FIFO: Add to the tail
        new_entry->prev = cache->tail;
        if (cache->tail != NULL)
        {
            cache->tail->next = new_entry;
        }
        cache->tail = new_entry;
        if (cache->head == NULL)
        {
            cache->head = new_entry;
        }
    }

    cache->size++;
}

// Cached Collatz steps function (for LRU/FIFO)
int cached_collatz_steps(Cache *cache, int number)
{
    CacheEntry *entry = find_cache_entry(cache, number);
    if (entry != NULL)
    {
        // Cache hit
        cache->hits++;
        if (cache->policy == CACHE_LRU)
        {
            move_to_head(cache, entry); // For LRU, move the entry to the head
        }
        return entry->steps;
    }

    // Cache miss
    cache->misses++;
    int steps = collatz_steps(number);
    add_to_cache(cache, number, steps);
    return steps;
}

// Calculate cache hit rate (for LRU/FIFO)
float get_cache_hit_rate(Cache *cache)
{
    int total_accesses = cache->hits + cache->misses;
    if (total_accesses == 0)
        return 0.0f;
    return ((float)cache->hits / total_accesses) * 100.0f;
}

// Free the cache (for LRU/FIFO)
void free_cache(Cache *cache)
{
    CacheEntry *current = cache->head;
    while (current != NULL)
    {
        CacheEntry *next = current->next;
        free(current);
        current = next;
    }
    free(cache);
}

// --------------------- ARC Cache Implementation ---------------------

// Create ARC Cache
ARC_Cache *create_arc_cache(int capacity)
{
    ARC_Cache *cache = (ARC_Cache *)malloc(sizeof(ARC_Cache));
    cache->T1 = NULL;
    cache->T2 = NULL;
    cache->B1 = NULL;
    cache->B2 = NULL;
    cache->capacity = capacity;
    cache->size = 0;
    cache->p = 0; // Initialize p
    return cache;
}

// Adjust ARC balance based on hits in B1/B2
void adjust_arc_balance(ARC_Cache *cache, int number)
{
    CacheEntry *b1_entry = find_cache_entry(cache->B1, number);
    CacheEntry *b2_entry = find_cache_entry(cache->B2, number);

    if (b1_entry != NULL)
    {
        cache->p = cache->capacity < cache->p + 1 ? cache->capacity : cache->p + 1; // Increase recency
        move_to_arc_head(&cache->T2, b1_entry);
    }
    else if (b2_entry != NULL)
    {
        cache->p = cache->p - 1 < 0 ? 0 : cache->p - 1; // Increase frequency
        move_to_arc_head(&cache->T2, b2_entry);
    }
    else
    {
        insert_arc_entry(cache, number, collatz_steps(number));
    }
}

// Insert new entry into ARC cache
void insert_arc_entry(ARC_Cache *cache, int number, int steps)
{
    evict_arc_entry(cache);

    CacheEntry *new_entry = (CacheEntry *)malloc(sizeof(CacheEntry));
    new_entry->number = number;
    new_entry->steps = steps;
    new_entry->next = NULL;
    new_entry->prev = NULL;

    if (cache->size < cache->p)
    {
        move_to_arc_head(&cache->T1, new_entry);
    }
    else
    {
        move_to_arc_head(&cache->T2, new_entry);
    }

    cache->size++;

    log_arc_state(cache);
}

// Evict an entry from ARC cache
void evict_arc_entry(ARC_Cache *cache)
{
    if (cache->T1 != NULL && cache->size > cache->p)
    {
        // Evict from T1
        CacheEntry *to_evict = cache->T1;
        cache->T1 = to_evict->next;
        if (cache->T1 != NULL)
        {
            cache->T1->prev = NULL;
        }
        // Add evicted entry to B1
        add_to_b1(cache, to_evict);
    }
    else if (cache->T2 != NULL)
    {
        // Evict from T2
        CacheEntry *to_evict = cache->T2;
        cache->T2 = to_evict->next;
        if (cache->T2 != NULL)
        {
            cache->T2->prev = NULL;
        }
        // Add evicted entry to B2
        add_to_b2(cache, to_evict);
    }

    // Decrement cache size
    cache->size--;

    log_arc_state(cache);
}

// Log ARC State
void log_arc_state(ARC_Cache *cache)
{
    printf("ARC State:\n");
    printf("T1 Size: %d, T2 Size: %d, B1 Size: %d, B2 Size: %d, p = %d\n",
           get_list_size(cache->T1), get_list_size(cache->T2),
           get_list_size(cache->B1), get_list_size(cache->B2), cache->p);
}