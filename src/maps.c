#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "extratypes.h"
#include "extrafuns.h"

extern int errno;

Map*** init_maps(Dictionary* bigdict, int max_word_size, int* words_count) {

    int* map_sizes = malloc(max_word_size * sizeof(int));
    for (int i = 1 ; i < max_word_size ; ++i) {
        map_sizes[i] = words_count[i] >> 5;
        if (words_count[i] & 0x1F) map_sizes[i]++;
    }

    Map*** maps = malloc(max_word_size * sizeof(Map**));
    if (maps == NULL) /* Malloc error handling */
        error("Error while allocating memory", errno);

    for (int i = 0 ; i < max_word_size ; ++i) {
        maps[i] = malloc((i + 2) * sizeof(Map*)); /* +2 because i is word_size - 1 */
        if (maps[i] == NULL) /* Malloc error handling */
            error("Error while allocating memory", errno);

        for (int j = 0 ; j <= i ; ++j) {
            maps[i][j] = malloc(26 * sizeof(Map));
            if (maps[i][j] == NULL) /* Malloc error handling */
                error("Error while allocating memory", errno);
        }

        maps[i][i + 1] = malloc(sizeof(Map));
        if (maps[i][i + 1] == NULL) /* Malloc error handling */
            error("Error while allocating memory", errno);
    }

    for (int word_size = 1 ; word_size < max_word_size ; ++word_size) { /* Starts from one because word length starts from 2 */
        for (int position = 0 ; position <= word_size ; ++position) { /* <= because word_size is -1 from actual length */
            for (int letter = 0 ; letter < 26 ; ++letter) {
                /* Calculating size */
                maps[word_size][position][letter].size = map_sizes[word_size];

                /* Allocating the array */
                maps[word_size][position][letter].array = calloc(map_sizes[word_size], sizeof(int));
                if (maps[word_size][position][letter].array == NULL) /* Calloc error handling */
                    error("Error while allocating memory", errno);
            }
        }
    }
    for (int i = 1 ; i < max_word_size ; ++i) {
        for (int j = 0 ; j < words_count[i] ; ++j) {
            char* word = bigdict[i][j];
            for (int k = 0 ; word[k] ; ++k) {
                /* Throw 1 to the appropriate bitmap */
                maps[i][k][word[k] - 'a'].array[j >> 5] |= 1 << (j & 0x1F);
            }
        }
    }
                
    for (int word_size = 1 ; word_size < max_word_size ; ++word_size) {
        /* Calculating size */
        maps[word_size][word_size + 1][0].size = map_sizes[word_size];

        /* Allocating the array */
        maps[word_size][word_size + 1][0].array = malloc(map_sizes[word_size] * sizeof(int));
        if (maps[word_size][word_size + 1][0].array == NULL) /* Malloc error handling */
            error("Error while allocating memory", errno);

        memset(maps[word_size][word_size + 1][0].array, 0xFF, map_sizes[word_size] * sizeof(int));

        if (words_count[word_size] & 0x1F) {
            for (int i = words_count[word_size] & 0x1F ; i < 32 ; ++i) {
                maps[word_size][word_size + 1][0].array[map_sizes[word_size] - 1] ^= 1 << i;
            }
        }
    }
    free(map_sizes);
    return maps;
}
//FIXME change after new word struct
void update_map(char** crossword, Word* word, Map*** maps) {
    Map* map = word->map;
    DBGCHECK(map->size == maps[word->size - 1][word->size]->size);
    memcpy(map->array, maps[word->size - 1][word->size]->array, map->size * sizeof(int));
    if (word->orientation) {
        for (int i = word->begin ; i <= word->end ; ++i) {
            char ch = crossword[i][word->constant];
            if (ch == '-') continue;
            join_map(map, maps[word->size - 1][i - word->begin] + (ch - 'a'));
        }
    }
    else {
        for (int i = word->begin ; i <= word->end ; ++i) {
            char ch = crossword[word->constant][i];
            if (ch == '-') continue;
            join_map(map, maps[word->size - 1][i - word->begin] + (ch - 'a'));
        }
    }
}

void join_map(Map* map1, Map* map2) {
    DBGCHECK(map1->size == map2->size); // debug tools
    register int* array1 = map1->array;
    register int* array2 = map2->array;
    int size = map1->size;
    for (register int i = 0 ; i < size ; ++i) {
        array1[i] &= array2[i];
    }
}

/* Brian Kernighan’s Algorithm */
int sum_bit(Map* map) {
    DBGCHECK(map != NULL); // debug tools
    register int* array = map->array;
    int size = map->size;
    register int sum = 0;
    for (register int i = 0 ; i < size ; ++i) {
        register int n = array[i];
        if (n == 0) continue;
        while (n) {
            n &= (n - 1);
            ++sum;
        }
    }
    map->sum = sum;
    return sum;
}