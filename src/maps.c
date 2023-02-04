#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "extratypes.h"
#include "extrafuns.h"

extern int errno;

Bitmaps init_maps(Dictionary* bigdict, int max_word_size, int* words_count, int** map_sizes_ret) {

    Bitmaps maps = malloc(max_word_size * sizeof(int***));
    if (maps == NULL) /* Malloc error handling */
        error("Error while allocating memory", errno);

    for (int i = 0 ; i < max_word_size ; ++i) {

        maps[i] = malloc((i + 2) * sizeof(int**)); /* +2 because i is word_size - 1 */
        if (maps[i] == NULL) /* Malloc error handling */
            error("Error while allocating memory", errno);

        for (int j = 0 ; j <= i ; ++j) {
            maps[i][j] = malloc(26 * sizeof(int*));
            if (maps[i][j] == NULL) /* Malloc error handling */
                error("Error while allocating memory", errno);
        }

        maps[i][i + 1] = malloc(sizeof(int*));
        if (maps[i][i + 1] == NULL) /* Malloc error handling */
            error("Error while allocating memory", errno);
    }
    
    /* Keeping track of map sizes */
    int* map_sizes = calloc(max_word_size, sizeof(int));
    if (map_sizes == NULL) /* Calloc error handling */
        error("Error while allocating memory", errno);

    for (int word_size = 1 ; word_size < max_word_size ; ++word_size) { /* Starts from one because word length starts from 2 */
        for (int position = 0 ; position <= word_size ; ++position) { /* <= because word_size is -1 from actual length */
            for (int letter = 0 ; letter < 26 ; ++letter) {

                /* Allocate memory */
                map_sizes[word_size] = words_count[word_size] >> 5;
                if (words_count[word_size] & 0x1F) map_sizes[word_size]++;
                maps[word_size][position][letter] = calloc(map_sizes[word_size], sizeof(int));
                if (maps[word_size][position][letter] == NULL) /* Calloc error handling */
                    error("Error while allocating memory", errno);
            }
        }
    }
    for (int i = 1 ; i < max_word_size ; ++i) {
        for (int j = 0 ; j < words_count[i] ; ++j) {
            char* word = bigdict[i][j];
            for (int k = 0 ; word[k] ; ++k) {
                /* Throw 1 to the appropriate bitmap */
                maps[i][k][word[k] - 'a'][j >> 5] |= 1 << (j & 0x1F);
            }
        }
    }
                
    for (int word_size = 1 ; word_size < max_word_size ; ++word_size) {

        *maps[word_size][word_size + 1] = malloc(map_sizes[word_size] * sizeof(int));
        if (maps[word_size][word_size + 1] == NULL) /* Malloc error handling */
            error("Error while allocating memory", errno);

        memset(*maps[word_size][word_size + 1], 0xFF, map_sizes[word_size] * sizeof(int));

        if (words_count[word_size] & 0x1F) {
            for (int i = words_count[word_size] & 0x1F ; i < 32 ; ++i) {  
                (*maps[word_size][word_size + 1])[map_sizes[word_size] - 1] ^= 1 << i;
            }
        }
    }

    *map_sizes_ret = map_sizes;
    return maps;
}

/**
 * @details debug tool
*/
void print_map(int* map, int map_size) {
    putchar('\n');
    for (int i = 0 ; i < map_size ; i+=8) {
        printf("%08x ", map[i]);
        if(i + 1 < map_size) printf("%08x ", map[i + 1]);
        if(i + 2 < map_size) printf("%08x ", map[i + 2]);
        if(i + 3 < map_size) printf("%08x ", map[i + 3]);
        if(i + 4 < map_size) printf("%08x ", map[i + 4]);
        if(i + 5 < map_size) printf("%08x ", map[i + 5]);
        if(i + 6 < map_size) printf("%08x ", map[i + 6]);
        if(i + 7 < map_size) printf("%08x ", map[i + 7]);
        putchar('\n');
    }
    putchar('\n');
}

void update_map(char** crossword, int* map, int map_size, Word word, Bitmaps maps) {
    memcpy(map, maps[word.size - 1][word.size][0], map_size * sizeof(int));
    if (word.orientation) {
        for (int i = word.begin ; i <= word.end ; ++i) {
            char ch = crossword[i][word.constant];
            if (ch != '-') {
                join_map(map, maps[word.size - 1][i - word.begin][ch - 'a'], map_size);
            }
        }
    }
    else {
        for (int i = word.begin ; i <= word.end ; ++i) {
            char ch = crossword[word.constant][i];
            if (ch != '-') {
                join_map(map, maps[word.size - 1][i - word.begin][ch - 'a'], map_size);
            }
        }
    }
}

void join_map(int* map1, int* map2, int map_size) {
    for (int i = 0 ; i < map_size ; ++i)
        map1[i] &= map2[i];
}

/* Brian Kernighan’s Algorithm */
int sum_bit(int* map, int map_size) {
    if(map == NULL) return 0;
    int count = 0;
    for (int i = 0 ; i < map_size ; ++i) {
        int n = map[i];
        if (n == 0) continue;
        while (n) {
            n &= (n - 1);
            ++count;
        }
    }
    return count;
}