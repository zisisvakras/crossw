#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "extratypes.h"
#include "extrafuns.h"

Bitmaps make_maps(Dictnode* dictionary, int max_word_size, int* words_count, int** map_sizes) {
    Bitmaps maps = malloc(max_word_size * sizeof(int***));
    for (int i = 0 ; i < max_word_size ; ++i) {
        maps[i] = malloc((i + 2) * sizeof(int**));
        for (int j = 0 ; j <= i ; ++j) {
            maps[i][j] = malloc(26 * sizeof(int*));
        }
        maps[i][i + 1] = malloc(sizeof(int*));
    }
    
    int* map_sizes_l = calloc(max_word_size, sizeof(int));
    for (int word_size = 0 ; word_size < max_word_size ; ++word_size) {
        for (int position = 0 ; position <= word_size ; ++position) {
            for (int letter = 0 ; letter < 26 ; ++letter) {

                /* Allocate memory */
                map_sizes_l[word_size] = words_count[word_size] / 32;
                if (words_count[word_size] % 32) map_sizes_l[word_size]++;
                maps[word_size][position][letter] = calloc(map_sizes_l[word_size], sizeof(int));

                /* Throw 1 to all found letters */
                for (int i = 0 ; i < words_count[word_size] ; ++i) {
                    if (dictionary[word_size][i].word[position] == 'a' + letter) {
                        maps[word_size][position][letter][i >> 5] |= 1 << (i & 0x1F);
                    }
                }
            }
        }
    }
    for (int word_size = 0 ; word_size < max_word_size ; ++word_size) {
        *maps[word_size][word_size + 1] = malloc(map_sizes_l[word_size] * sizeof(int));
        for (int i = 0 ; i < map_sizes_l[word_size] ; ++i) {
            (*maps[word_size][word_size + 1])[i] |= 0xFFFFFFFF;
        }
        if (words_count[word_size] % 32) {
            for (int i = words_count[word_size] % 32 ; i < 32 ; ++i) {  
                (*maps[word_size][word_size + 1])[map_sizes_l[word_size] - 1] ^= 1 << i;
            }
        }
    }

    *map_sizes = map_sizes_l;
    return maps;
}

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

int* create_map(Bitmaps maps, int* map_sizes, char* filter) {
    int word_size = strlen(filter);
    int map_size = map_sizes[word_size - 1];
    int* map = malloc(map_size * sizeof(int));
    memcpy(map, maps[word_size - 1][word_size][0], map_size * sizeof(int));
    for (int i = 0 ; i < word_size ; ++i) {
        if (filter[i] != '?') {
            join_map(map, maps[word_size - 1][i][filter[i] - 'a'], map_size);
        }
    }
    return map;
}

void join_map(int* map1, int* map2, int map_size) {
    for (int i = 0 ; i < map_size ; ++i) {
        map1[i] &= map2[i];
    }
}

int sum_bit(int* map, int map_size) {
    if(map == NULL) return 0;
    int count = 0;
    for (int i = 0 ; i < map_size ; ++i) {
        if (map[i] == 0) continue;
        for (int j = 0 ; j < 32 ; ++j) {
            if ((map[i] >> j) & 1) ++count;
        }
    }
    return count;
}