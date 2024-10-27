/**********************************************************************/
/*   NAME: Osaze Ogieriakhi                                           */
/*   DATE: 09.26.204                                                  */
/*   PROJECT: Cache access simulation                                 */
/*   FILE: function library "cachefunctions.h"                        */
/**********************************************************************/
#include "cachesim.h"
int misses = 0 , hits = 0, evictions = 0;
unsigned long current_time = 0;

void print_summary(int hits, int misses, int evictions)
{
    printf("hits:%d misses:%d evictions:%d\n", hits, misses, evictions);
}


void print_usage(char* argv[])
{
    printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");
    printf("\nExamples:\n");
    printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/trace01.dat\n", argv[0]);
    printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/trace01.dat\n", argv[0]);
    exit(0);
}


// cache* initiateCache(int setCount, int lineCount)
// {
//     cache* newCache = (cache*)malloc(sizeof(cache));

//     newCache->set_count = setCount;
//     newCache->line_count = lineCount;


//     newCache->sets = (cacheSet*) malloc(setCount * sizeof(cacheSet));

//     if (newCache->sets == NULL) {
//     printf("Memory allocation failed for sets\n");
//     exit(EXIT_FAILURE);
//     }

//     for(int i = 0; i < setCount; i++) //Allocating memory for each line in each set
//     {
//         newCache->sets[i].lines = (cacheLine*) malloc(lineCount * sizeof(cacheLine));

//         if (newCache->sets[i].lines == NULL) {
//         printf("Memory allocation failed for lines in set %d\n", i);
//         exit(EXIT_FAILURE);
//         }

//         for(int j = 0; j < lineCount; j++)
//         {
//             newCache->sets[i].lines[j].valid = 0;     // Set valid bit to 0 (invalid)
//             newCache->sets[i].lines[j].tag = 0;       // Initialize tag to 0
//             newCache->sets[i].lines[j].lru_counter = 0; // Initialize the lru to 0
//         }
//     }

//     return newCache;
// }

cache* initializeCache(int numSets, int E) {
    cache* newCache = (cache*)malloc(sizeof(cache));
    newCache->sets = (cacheSet*)malloc(numSets * sizeof(cacheSet));

    // Initialize each cache set and allocate memory for its lines
    for (int i = 0; i < numSets; i++) {
        newCache->sets[i].lines = (cacheLine*)malloc(E * sizeof(cacheLine));

         if (newCache->sets[i].lines == NULL) {
            printf("Error: Could not allocate memory for lines in set %d\n", i);
            exit(EXIT_FAILURE);
        }

        // Initialize each cache line
        for (int j = 0; j < E; j++) {
            newCache->sets[i].lines[j].valid = 0;
            newCache->sets[i].lines[j].tag = 0;
            newCache->sets[i].lines[j].lru_counter = 0;
            newCache->sets[i].lines[j].lastUsed = 0;
        }
    }

    return newCache;
}

void freeCache(cache* deadCache)
{
    for (int i = 0; i < deadCache->set_count; i++) {
        free(deadCache->sets[i].lines);
    }
    
    // Free the sets
    free(deadCache->sets);

    // Free the cache structure itself
    free(deadCache);
}


int findLRU(cacheSet *set, int E) {
    int lruIndex = 0;
    unsigned long oldestTime = set->lines[0].lastUsed;

    for (int i = 1; i < E; i++) {
        if (set->lines[i].lastUsed < oldestTime) {
            oldestTime = set->lines[i].lastUsed;
            lruIndex = i;
        }
    }
    return lruIndex;
}


void parseTrace(const char* tracefile, cache *cache, int s, int E, int b, int verbose, int* hitCount, int* missCount, int* evictionCount) {
    // Open the file
    FILE* file = fopen(tracefile, "r");

    if (!file) {
        printf("Oops, trace file: %s could not be opened\n", tracefile);
        exit(EXIT_FAILURE);
    }

    

    char operation;
    unsigned long address; // Memory address in hexadecimal.
    int size;              // Number of bytes

    char line[256];

    // Loop over each line in the trace file
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'I') {
            continue;  // Skip instruction accesses
        }

        // Parse the operation, address, and size
        sscanf(line, " %c %lx,%d", &operation, &address, &size);

        if (verbose) {
            // printf("Operation: %c, Address: 0x%lx, Size: %d\n", operation, address, size);
        }

        // Perform the corresponding cache operation
        switch (operation) {
            case 'L': // Load
                accessCache('L', address, size, cache, s, E, b, verbose, hitCount, missCount, evictionCount);
                break;
            case 'S': // Store
                accessCache('S', address, size, cache, s, E, b, verbose, hitCount, missCount, evictionCount);
                break;
            case 'M': // Modify (Load + Store)
                accessCache('L', address, size, cache, s, E, b, verbose, hitCount, missCount, evictionCount); // First a load
                accessCache('S', address, size, cache, s, E, b, verbose, hitCount, missCount, evictionCount); // Then a store
                break;
            default:
                printf("Unknown operation: %c\n", operation);
                break;
        }
    }

    fclose(file);
}



void accessCache(char operation, unsigned long address, int size, cache* cache, int s, int E, int b, int verbose,
                int* hitCount, int* missCount, int* evictionCount)
{
    // Calculate the set index and tag
    unsigned long setIndex = (address >> b) & ((1 << s) - 1);
    unsigned long tag = address >> (s + b);


    cacheSet* set = &cache->sets[setIndex];
    int hit = 0;
    int emptyIndex = -1;
    int numSets = 1 << s;

    // Boundary check for set index
    if (setIndex >= numSets) {
        printf("Error: Invalid SetIndex: %ld, NumSets: %d\n", setIndex, numSets);
        exit(EXIT_FAILURE);
    }

    if (set == NULL || set->lines == NULL) {
        printf("Error: Set or line not initialized\n");
        exit(EXIT_FAILURE);
    }



    // Iterate over cache lines in the set
    for (int i = 0; i < E; i++) {
       // printf("Iteration: %d, Valid: %d, Tag: %lx\n", i, set->lines[i].valid, set->lines[i].tag);

        if (set->lines[i].valid && set->lines[i].tag == tag) // found the right data
        {
            hit = 1;
            set->lines[i].lastUsed = current_time++;
            if (verbose) printf("%c %lx,%d hit\n", operation, address, size);
            break;
        }

        // Track the first empty line (in case of miss)
        if (!set->lines[i].valid && emptyIndex == -1) {
            emptyIndex = i;
        }
    }


    if (!hit) {
        (*missCount)++;
        if (verbose) printf("%c %lx, %d miss\n", operation, address, size);

        // If no empty line, perform eviction
        if (emptyIndex == -1) {
            int lruIndex = findLRU(set, E);
            emptyIndex = lruIndex; // Use LRU line for eviction
            (*evictionCount)++;
            if (verbose) printf("%c %lx, %d eviction\n", operation, address, size);
        }

        // Insert new cache line
        set->lines[emptyIndex].valid = 1;
        set->lines[emptyIndex].tag = tag;
        set->lines[emptyIndex].lastUsed = current_time++;
    } else {
        (*hitCount)++;
    }

    // If the operation is 'M', treat it as a second access (store after load)
    if (operation == 'M') {
        (*hitCount)++;
        if (verbose) printf("M %lx,%d hit\n", address, size);
    }
}
