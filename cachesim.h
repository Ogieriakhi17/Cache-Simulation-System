/**********************************************************************/
/*   NAME: Osaze Ogieriakhi                                           */
/*   DATE: 09.26.204                                                  */
/*   PROJECT: Cache access simulation                                 */
/*   FILE: header file "cachesim.h"                                   */
/**********************************************************************/


#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>


typedef struct {
    int valid;
    unsigned long tag;
    int lru_counter;
    unsigned long lastUsed;
}cacheLine;

typedef struct {
    cacheLine* lines; // the number of lines in a set;
}cacheSet;


typedef struct {
    cacheSet *sets;   // Array of cache sets
    int set_count;       // Number of sets
    int line_count;      // Number of lines per set (associativity)
} cache;



/* 
 * this function provides a standard way for your cache
 * simulator to display its final statistics (i.e., hit and miss)
 */ 
void print_summary(int hits, int misses, int evictions);

/*
 * print usage info
 */
void print_usage(char* argv[]);

/* initializeCache()
 *  Functionality: Creates a new cache and initializes the locations
 *  Arguments: the new values for the size of the sets and lines 
 *  Return: pointer to the new cache
 */
cache* initializeCache(int numSets, int E);

/* freeCache()
 *  Functionality: Deleting a cache
 *  Arguments: Pointer to the target Cache
 *  Return: NONE
 */
void freeCache(cache* deadCache);

/* parseTrace()
 *  Functionality: Parse the trace file
 *  Arguments: Nameo of the trace file and verbose value
 *  Return: NONE
 */
void parseTrace(const char* tracefile, cache* cache, int s, int E, int b, int verbose, int* hitCount, int* missCount, int* evictionCount);

/* accessCache()
 *  Functionality: To access the cache and determine hits and misses
 *  Arguments: Cache operation, address, size and specified cache pointer
 *  Return: NONE
 */
void accessCache(char operation, unsigned long address, int size, cache* cache, int s, int E, int b, int verbose,
                int* hitCount, int* missCount, int* evictionCount);

/* findLRU()
 *  Functionality: Helper function designed to facilitate the Last Recently Used policy of
                    the cache.
 *  Arguments: The target cache and its line size.
 *  Return: the index of the last recently used cache block
 */
int findLRU(cacheSet *set, int E);