#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "extratypes.h"
#include "extrafuns.h"

extern int errno;

int main(int argc, char** argv) {

    /* This program cannot support ints lower than 4 bytes ;-; */
    if (sizeof(int) < 4) {
        fprintf(stderr, "I can't run on this machine ;-;\n");
        return 1;
    }

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

    /* Map the crossword */
    int grid_count = count_words_on_grid(crossword, crossword_size); /* Counts words on grid */
    Word** grid_words = map_words_on_grid(crossword, crossword_size, grid_count);

    /* Initialize dictionaries */
    int* dict_count = NULL; /* Counts words in each dictionary */
    Dictionary* bigdict = init_dictionary(dictionary_path, max_word_size, &dict_count);

    /* Initialize dict_maps */
    Map*** dict_maps = init_dict_maps(bigdict, max_word_size, dict_count);
    
    /* Initializing maps for all words */
    for (int i = 0 ; i < grid_count ; ++i) {
        Map* src = dict_maps[grid_words[i]->size - 1][grid_words[i]->size];
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
        solve_crossword(&crossword, crossword_size, bigdict, grid_words, grid_count, dict_maps);
        if (draw_mode) draw_crossword(crossword, crossword_size);
        else print_solution(crossword, ord_words, grid_count);
    }

    /* Cleanup */
    free_dictionary(bigdict, max_word_size, dict_count);
    free(dict_count);
    return 0;
}