/**********************************************************************/
/*   NAME: Osaze Ogieriakhi                                           */
/*   DATE: 09.26.204                                                  */
/*   PROJECT: Cache access simulation                                 */
/*   FILE: main file "cachesim.h"                                     */
/**********************************************************************/

#include "cachesim.h"

#define ADDRESS_LENGTH 64  // 64-bit memory addressing


int hit_count = 0;
int miss_count = 0;
int eviction_count = 0;


int main(int argc, char *argv[]) {
    int s = 0, E = 0, b = 0;
    int verbose = 0;
    char *tracefile = NULL;

    int * hits = &hit_count, *misses = &miss_count, *evictions = &eviction_count;
    // Parse command-line arguments using getopt
    int opt;
    while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch (opt) {
            case 'h':
                print_summary(hit_count, miss_count, eviction_count);
                exit(EXIT_SUCCESS);
            case 'v':
                verbose = 1;
                break;
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                tracefile = optarg;
                break;
            default:
                print_usage(argv);
                printf("youre cooked\n");
                exit(EXIT_FAILURE);
        }
    }


    if (!tracefile) {
        printf("Error: Missing tracefile.\n");
        print_usage(argv);
        printf("you're cooked\n");
        exit(EXIT_FAILURE);
    }

    int setCount = 1 << s;

    // Initialize the cache
    cache *myCache = initializeCache(setCount, E);



    // Parse the trace file and simulate the cache
    parseTrace(tracefile, myCache, s, E, b, verbose, hits, misses, evictions);

    // Cleanup and print summary
    freeCache(myCache);
    print_summary(hit_count, miss_count, eviction_count);

    return 0;
}

