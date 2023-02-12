#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "extratypes.h"
#include "extrafuns.h"

extern int errno;

Map*** init_dict_maps(Dictionary* bigdict, int max_word_size, int* words_count) {
    /* Calculating map_sizes */
    int* map_sizes = malloc(max_word_size * sizeof(int));
    mallerr(map_sizes, errno);
    for (int i = 1 ; i < max_word_size ; ++i) {
        map_sizes[i] = words_count[i] >> 5;
        /* If words are not divisible by 32 we need 1 extra non-full array */
        if (words_count[i] & 0x1F) map_sizes[i]++;
    }

    /* Allocating memory for maps */
    Map*** maps = malloc(max_word_size * sizeof(Map**));
    mallerr(maps, errno);
    for (int i = 0 ; i < max_word_size ; ++i) {
        maps[i] = malloc((i + 2) * sizeof(Map*)); /* +2 because i is word_size - 1 */
        mallerr(maps[i], errno);
        for (int j = 0 ; j <= i ; ++j) {
            maps[i][j] = malloc(26 * sizeof(Map)); /* 26 letters of the english alphabet */
            mallerr(maps[i][j], errno);
        }
        /* Adding an extra map that will be full of 1s */
        maps[i][i + 1] = malloc(sizeof(Map));
        mallerr(maps[i][i + 1], errno);
    }

    /* Allocating the arrays */
    /* Starts from one because word length starts from 2 */
    for (int word_size = 1 ; word_size < max_word_size ; ++word_size) {
        /* <= because word_size is -1 from actual length */
        for (int position = 0 ; position <= word_size ; ++position) {
            for (int letter = 0 ; letter < 26 ; ++letter) {
                maps[word_size][position][letter].size = map_sizes[word_size];
                maps[word_size][position][letter].array = calloc(map_sizes[word_size], sizeof(int));
                mallerr(maps[word_size][position][letter].array, errno);
            }
        }
    }
    /* Creating the maps */
    for (int i = 1 ; i < max_word_size ; ++i) {
        for (int j = 0 ; j < words_count[i] ; ++j) { /* Looping through every word in dict */
            char* word = bigdict[i][j];
            for (int k = 0 ; word[k] ; ++k) { /* Every letter in word */
                /**
                 * Throw 1 to the appropriate bitmap.
                 * j >> 5 calculates the 32 bit int that the 1 will put in.
                 * j & 0x1F (j mod 31) calculates the position in the int.
                 * We shift the bit to the position and we put it in with |.
                 * i is the word_length.
                 * k is the position in the word.
                */
                maps[i][k][word[k] - 'a'].array[j >> 5] |= 1 << (j & 0x1F);
            }
        }
    }
    
    /* Creating the maps with all 1s */
    for (int word_size = 1 ; word_size < max_word_size ; ++word_size) {
        maps[word_size][word_size + 1][0].size = map_sizes[word_size];
        maps[word_size][word_size + 1][0].array = malloc(map_sizes[word_size] * sizeof(int));
        mallerr(maps[word_size][word_size + 1][0].array, errno);
        /* Setting the whole array with 1s */
        memset(maps[word_size][word_size + 1][0].array, 0xFF, map_sizes[word_size] * sizeof(int));
        /* We need to remove the excess 1s in the case of extra int (to cover non divisible word_counts) */
        if (words_count[word_size] & 0x1F) {
            int remove = 0xFFFFFFFF;
            remove <<= words_count[word_size] & 0x1F; /* Skip all that is fine */
            maps[word_size][word_size + 1][0].array[map_sizes[word_size] - 1] ^= remove; /* Remove rest bits */
        }
    }
    free(map_sizes);
    return maps;
}
//FIXME change after new word struct
void update_map(char** crossword, Map* map, Word* word, Map*** maps) {
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