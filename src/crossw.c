#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "extratypes.h"
#include "extrafuns.h"

extern int errno;

//FIXME update makefile
int main(int argc, char** argv) {

    if (argc < 2) { /* Argument error handling */
        fprintf(stderr, "Not enough arguments\n");
        return 1;
    }

    char* dictionary_path = "Words.txt";
    char* crossword_path = argv[1];
    int check_mode = 0, draw_mode = 0;

    while (--argc) {
        if (argv[argc]) {
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

    /* Intialize the crossword part */
    char** crossword = NULL;
    int max_word_size, crossword_size;
    init_crossword(crossword_path, &crossword, &crossword_size, &max_word_size);

    /* Initialize dictionaries */
    int* words_count = NULL;
    Dictionary* bigdict = init_dictionary(dictionary_path, max_word_size, &words_count);

    /* Initialize maps */
    int* map_sizes = NULL;
    Bitmaps maps = init_maps(bigdict, max_word_size, words_count, &map_sizes);

    int wordnode_count = 0;
    Wordnode words = map_words(crossword, crossword_size, &wordnode_count);

    if (check_mode) {
    //     int ret_check = check_crossword(crossword, crossword_size, words, wordnode_count, bigdict, maps, map_sizes);
    //     if (draw_mode) draw_crossword(crossword, crossword_size);
        return 0;
    }
    //print_dict(bigdict, max_word_size);
    //print_words(words, wordnode_count);
    solve_crossword(&crossword, crossword_size, bigdict, words, wordnode_count, maps, map_sizes);
    if (draw_mode) draw_crossword(crossword, crossword_size);
    else print_solution(crossword, crossword_size);
    free_dictionary(bigdict, max_word_size, words_count);
    printf("dict: %s cross: %s max: %d\n", dictionary_path, crossword_path, max_word_size);
    return 0;
}