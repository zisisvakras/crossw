#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "extratypes.h"
#include "extrafuns.h"

extern int errno;

int main(int argc, char** argv) {

    if (argc < 2) { /* Argument error handling */
        fprintf(stderr, "Not enough arguments\n");
        return 1;
    }

    /* Initiliazing argument flags to default values */
    char* dictionary_path = "Words.txt";
    char* crossword_path = argv[1];
    int check_mode = 0, draw_mode = 0;

    /* Loop reading arguments */
    while (--argc) {
        if (argv[argc]) {
            /* Setting the flags as needed */
            if (!strcmp(argv[argc], "-dict")) {
                if(argv[argc + 1] == NULL) { /* Argument error handling */
                    fprintf(stderr, "Could not find dictionary\n");
                    return 1;
                }
                dictionary_path = argv[argc + 1];
            }
            if (!strcmp(argv[argc], "-check")) {
                check_mode = 1;
            }
            if (!strcmp(argv[argc], "-draw")) {
                draw_mode = 1;
            }
        }
    }

    /* Intialize the crossword grid */
    char** crossword = NULL;
    int max_word_size, crossword_size;
    init_crossword(crossword_path, &crossword, &crossword_size, &max_word_size);

    /* Allocating multipliers */
    int** multi = malloc(max_word_size * sizeof(int*));
    mallerr(multi, errno);
    for (int i = 0 ; i < max_word_size ; ++i) {
        multi[i] = calloc((i + 1), sizeof(int));
        mallerr(multi[i], errno);
    }

    /* Map the crossword */
    int grid_count = count_words_on_grid(crossword, crossword_size); /* Counts words on grid */
    Word** grid_words = map_words_on_grid(crossword, crossword_size, grid_count, multi);

    /* Initialize dictionaries */
    int*** worth = NULL;
    int* dict_count = NULL; /* Counts words in each dictionary */
    Dictionary* bigdict = init_dictionary(dictionary_path, max_word_size, &dict_count, multi, &worth);

    int* map_sizes = malloc(max_word_size * sizeof(int));
    mallerr(map_sizes, errno);
    for (int i = 1 ; i < max_word_size ; ++i) {
        map_sizes[i] = dict_count[i] >> 5;
        /* If words are not divisible by 32 we need 1 extra non-full array */
        if (dict_count[i] & 0x1F) map_sizes[i]++;
    }


    /* Initializing maps for all words */
    for (int i = 0 ; i < grid_count ; ++i) {
        int ind = grid_words[i]->size - 1;
        grid_words[i]->dict = malloc(dict_count[ind] * sizeof(char*));
        Dictionary dict = grid_words[i]->dict;
        memcpy(grid_words[i]->dict, bigdict[ind], dict_count[ind] * sizeof(char*));

        int* word_values = calloc(dict_count[ind], sizeof(int));
        for (int j = 0 ; j < grid_words[i]->insecc ; ++j) {
            int len = grid_words[i]->insecs[j].len;
            int pos = grid_words[i]->insecs[j].pos;
            int pos_local = grid_words[i]->insecs[j].pos_local;
            for (int k = 0 ; k < dict_count[ind] ; ++k)
                word_values[k] += worth[len - 1][pos][(int)dict[k][pos_local]]
                                * worth[len - 1][pos][(int)dict[k][pos_local]]
                                //* multi[ind][pos_local] //;
                                * grid_words[i]->insecs[j].word->insecc
                                * grid_words[i]->insecs[j].word->insecc
                                * grid_words[i]->insecs[j].word->insecc;
        }
        sort_dictionary(dict, word_values, 0, dict_count[ind] - 1);
        free(word_values);
        /* Bitmaps section */
        grid_words[i]->pre_maps = malloc((ind + 2) * sizeof(Map*)); /* +2 because i is word_size - 1 */
        Map** pre_maps = grid_words[i]->pre_maps; 
        for (int j = 0 ; j <= ind + 1 ; ++j) {
            pre_maps[j] = malloc(256 * sizeof(Map));
        }
        /* Adding an extra map that will be full of 1s */
        pre_maps[ind + 1] = malloc(sizeof(Map));
        for (int position = 0 ; position <= ind ; ++position) {
            for (int letter = 0 ; letter < 256 ; ++letter) {
                pre_maps[position][letter].size = map_sizes[ind];
                pre_maps[position][letter].array = calloc(map_sizes[ind], sizeof(int));
            }
        }
        for (int j = 0 ; j < dict_count[ind] ; ++j) { /* Looping through every word in dict */
            char* word = dict[j];
            for (int k = 0 ; word[k] ; ++k) {
                pre_maps[k][(int)word[k]].array[j >> 5] |= 1 << (j & 0x1F);
            }
        }
        pre_maps[ind + 1][0].size = map_sizes[ind];
        pre_maps[ind + 1][0].array = malloc(map_sizes[ind] * sizeof(int));
        memset(pre_maps[ind + 1][0].array, 0xFF, map_sizes[ind] * sizeof(int));
        /* We need to remove the excess 1s in the case of extra int (to cover non divisible word_counts) */
        if (dict_count[ind] & 0x1F) {
            int remove = 0xFFFFFFFF;
            remove <<= dict_count[ind] & 0x1F; /* Skip all that is fine */
            pre_maps[ind + 1][0].array[map_sizes[ind] - 1] ^= remove; /* Remove rest bits */
        }
    }
    free(map_sizes);

    /* Initialize dict_maps */
    Map*** dict_maps = init_dict_maps(bigdict, max_word_size, dict_count);
    
    /* Initializing maps for all words */
    for (int i = 0 ; i < grid_count ; ++i) {
        Map* src = grid_words[i]->pre_maps[grid_words[i]->size];
        grid_words[i]->map = malloc(sizeof(Map));
        grid_words[i]->map->size = src->size;
        grid_words[i]->map->array = malloc(src->size * sizeof(int));
        /* Copying the map with 1s */
        memcpy(grid_words[i]->map->array, src->array, src->size * sizeof(int));
        sum_bit(grid_words[i]->map);
    }

    /* Making an array of words thats in the proper order for check and print */
    Word** ord_words = malloc(grid_count * sizeof(Word*));
    mallerr(ord_words, errno);
    int ord_i = 0;
    for (int i = 0 ; i < grid_count ; ++i) {
        if (!(grid_words[i]->orientation)) {
            ord_words[ord_i++] = grid_words[i];
        }
    }
    for (int i = 0 ; i < grid_count ; ++i) {
        if (grid_words[i]->orientation) {
            ord_words[ord_i++] = grid_words[i];
        }
    }

    /* Program behavior based on flags */
    if (check_mode) {
        check_crossword(crossword, ord_words, dict_maps, grid_count);
        if (draw_mode) draw_crossword(crossword, crossword_size);
        printf("dict: %s cross: %s max: %d words: %d\n", dictionary_path, crossword_path, max_word_size, grid_count); //FIXME remove
    }
    else {
        solve_crossword(&crossword, crossword_size, grid_words, grid_count);
        if (draw_mode) draw_crossword(crossword, crossword_size);
        else print_solution(crossword, ord_words, grid_count);
    }

    /* Cleanup */
    free_dictionary(bigdict, max_word_size, dict_count);
    free(dict_count);
    return 0;
}