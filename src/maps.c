#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "extratypes.h"
#include "extrafuns.h"

extern int errno;

Map*** init_dict_maps(Dictionary* bigdict, int max_word_size, int* words_count,
                      int* lengths_on_grid, int* ascii_on_dict) {

    /* Calculating map_sizes */
    int* map_sizes = malloc(max_word_size * sizeof(int));
    mallerr(map_sizes, errno);
    for (int i = 1 ; i < max_word_size ; ++i) {
        if (lengths_on_grid[i] == 0) continue;
        map_sizes[i] = words_count[i] >> 6;
        /* If words are not divisible by 64 we need 1 extra non-full bit array (extra long long) */
        if (words_count[i] & 0x3F) ++map_sizes[i];
    }

    /* Allocating memory for maps */
    Map*** maps = calloc(max_word_size, sizeof(Map**));
    mallerr(maps, errno);
    for (int i = 1 ; i < max_word_size ; ++i) {
        if (lengths_on_grid[i] == 0) continue;
        maps[i] = calloc((i + 2), sizeof(Map*)); /* +2 because i is word_size - 1 */
        mallerr(maps[i], errno);
        for (int j = 0 ; j <= i ; ++j) {
            maps[i][j] = calloc(256, sizeof(Map)); /* all of ascii table */
            mallerr(maps[i][j], errno);
        }
        /* Adding an extra map that will be full of 1s */
        maps[i][i + 1] = calloc(1, sizeof(Map));
        mallerr(maps[i][i + 1], errno);
    }

    /* Allocating the arrays */
    /* Starts from one because word length starts from 2 */
    for (int word_size = 1 ; word_size < max_word_size ; ++word_size) {
        if (lengths_on_grid[word_size] == 0) continue;
        /* <= because word_size is -1 from actual length */
        for (int position = 0 ; position <= word_size ; ++position) {
            for (int letter = 0 ; letter < 256 ; ++letter) {
                if (ascii_on_dict[letter] == 0) continue;
                maps[word_size][position][letter].size = map_sizes[word_size];
                maps[word_size][position][letter].array = calloc(map_sizes[word_size], sizeof(long long));
                mallerr(maps[word_size][position][letter].array, errno);
            }
        }
    }
    /* Creating the maps */
    for (int i = 1 ; i < max_word_size ; ++i) {
        if (lengths_on_grid[i] == 0) continue;
        register Map** maps_i = maps[i];
        for (int j = 0 ; j < words_count[i] ; ++j) { /* Looping through every word in dict */
            register char* word = bigdict[i][j];
            int offset = j >> 6;
            register long long ap_bit = 1LL << (j & 0x3F); /* Bit to change in bitmap */
            for (int k = 0 ; k <= i ; ++k) { /* Every letter in word */
                /**
                 *  Throw 1 to the appropriate bitmap.
                 *  offset is the 64 bit long long that the 1 will be put in.
                 *  ap_bit is the position in the long long.
                 *  We shift the bit to the position and we put it in with |.
                 *  i is the word_length.
                 *  k is the position in the word.
                */
                maps_i[k][(int)word[k]].array[offset] |= ap_bit;
            }
        }
    }
    
    /* Creating the maps with all 1s */
    for (int word_size = 1 ; word_size < max_word_size ; ++word_size) {
        if (lengths_on_grid[word_size] == 0) continue;
        maps[word_size][word_size + 1][0].size = map_sizes[word_size];
        maps[word_size][word_size + 1][0].array = malloc(map_sizes[word_size] * sizeof(long long));
        mallerr(maps[word_size][word_size + 1][0].array, errno);
        /* Setting the whole array with 1s */
        memset(maps[word_size][word_size + 1][0].array, 0xFF, map_sizes[word_size] * sizeof(long long));
        /* We need to remove the excess 1s in the case of extra long long (to cover non divisible word_counts) */
        if (words_count[word_size] & 0x3F) {
            long long remove = 0xFFFFFFFFFFFFFFFFLL; /* -1 but this looks cooler */
            remove <<= words_count[word_size] & 0x3F; /* Skip all that is fine */
            maps[word_size][word_size + 1][0].array[map_sizes[word_size] - 1] ^= remove; /* Remove rest bits */
        }
    }
    free(map_sizes);
    return maps;
}

void free_maps(Map*** maps, int max_word_size) {
    for (int word_size = 1 ; word_size < max_word_size ; ++word_size) {
        if (!maps[word_size]) continue; /* Some maps might not be allocated */
        for (int position = 0 ; position <= word_size ; ++position) {
            for (int letter = 0 ; letter < 256 ; ++letter) {
                free(maps[word_size][position][letter].array);
            }
            free(maps[word_size][position]);
        }
        free(maps[word_size][word_size + 1][0].array);
        free(maps[word_size][word_size + 1]);
        free(maps[word_size]);
    }
    free(maps);     
}

/* Bitwise and two bit arrays */
void join_map(Map* map1, Map* map2) {
    register long long* array1 = map1->array;
    register long long* array2 = map2->array;
    register int size = map1->size;
    while (size--) {
        *array1++ &= *array2++;
    }
}

/* Check if some & operation will result in empty domain */
int fc_check(Map* map1, Map* map2) {
    register long long* array1 = map1->array;
    register long long* array2 = map2->array;
    register int size = map1->size;
    while (size--) {
        if (*array1++ & *array2++) return 0;
    }
    return 1;
}

/* Calculating the number of 1s in bit array (domain size) */
int sum_bit(Map* map) {
    register long long* array = map->array;
    register int size = map->size;
    register int sum = 0;
    /* Brian Kernighan’s Algorithm */
    while (size--) {
        register long long n = array[size];
        if (n == 0) continue;
        while (n) {
            n &= (n - 1);
            ++sum;
        }
    }
    map->sum = sum;
    return sum;
}