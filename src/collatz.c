#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "collatz.h"
#include "cache.h"
#include "config.h"

// Define the collatz_steps function here
int collatz_steps(int n)
{
    int steps = 0;
    while (n != 1)
    {
        if (n % 2 == 0)
        {
            n /= 2;
        }
        else
        {
            n = 3 * n + 1;
        }
        steps++;
    }
    return steps;
}

int main(int argc, char *argv[])
{
    if (argc != 6 && argc != 7)
    {
        printf("Usage: %s N MIN MAX cache_policy cache_size [logging]\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    int MIN = atoi(argv[2]);
    int MAX = atoi(argv[3]);
    char *cache_policy = argv[4];
    int cache_size = atoi(argv[5]);
    int logging_enabled = 0; // Default to no logging
    if (argc == 7)
    {
        logging_enabled = atoi(argv[6]); // Use logging if 6th argument is provided
    }

    // Initialize cache based on policy
    Cache *cache = NULL;
    ARC_Cache *arc_cache = NULL;

    if (strcmp(cache_policy, "ARC") == 0)
    {
        arc_cache = create_arc_cache(cache_size);
    }
    else
    {
        cache = create_cache(cache_size, cache_policy);
    }

    srand(time(NULL));
    for (int i = 0; i < N; i++)
    {
        int rand_num = (rand() % (MAX - MIN + 1)) + MIN;
        int steps;

        if (arc_cache != NULL)
        {
            steps = cached_collatz_steps_arc(arc_cache, rand_num);
        }
        else
        {
            steps = cached_collatz_steps(cache, rand_num);
        }

        printf("%d, %d\n", rand_num, steps);
    }

    // Print cache hit rate
    if (arc_cache != NULL)
    {
        printf("Cache hit rate: %.2f%%\n", get_cache_hit_rate_arc(arc_cache));
        if (logging_enabled)
        {
            log_arc_state(arc_cache); // Log ARC state if logging is enabled
        }
    }
    else
    {
        printf("Cache hit rate: %.2f%%\n", get_cache_hit_rate(cache));
    }

    // Clean up cache
    if (arc_cache != NULL)
    {
        free_arc_cache(arc_cache);
    }
    else
    {
        free_cache(cache);
    }

    return 0;
}