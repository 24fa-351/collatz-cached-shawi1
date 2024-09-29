#ifndef CACHE_H
#define CACHE_H

// Original Cache Entry for LRU/FIFO
typedef struct CacheEntry
{
    int number;
    int steps;
    struct CacheEntry *next;
    struct CacheEntry *prev;
} CacheEntry;

// Original Cache for LRU/FIFO
typedef struct Cache
{
    int capacity;
    int size;
    int hits;
    int misses;
    int policy;
    CacheEntry *head;
    CacheEntry *tail;
} Cache;

// ARC Cache structure
typedef struct
{
    CacheEntry *T1; // Recency-based LRU list
    CacheEntry *T2; // Frequency-based LRU list
    CacheEntry *B1; // Ghost cache for entries evicted from T1
    CacheEntry *B2; // Ghost cache for entries evicted from T2
    int p;          // Adaptive parameter to balance between T1 and T2
    int capacity;   // Cache capacity
    int size;       // Current cache size
    int hits;
    int misses;
} ARC_Cache;

// Function declarations for LRU/FIFO
Cache *create_cache(int capacity, const char *policy);
int cached_collatz_steps(Cache *cache, int number);
float get_cache_hit_rate(Cache *cache);
void free_cache(Cache *cache);

// Function declarations for ARC
ARC_Cache *create_arc_cache(int capacity);
void adjust_arc_balance(ARC_Cache *cache, int number);
void insert_arc_entry(ARC_Cache *cache, int number, int steps);
void evict_arc_entry(ARC_Cache *cache);
void log_arc_state(ARC_Cache *cache);

#endif // CACHE_H