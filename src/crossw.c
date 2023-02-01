#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "extratypes.h"
#include "extrafuns.h"

extern int errno;

int main(int argc, char** argv) {

    //TODO better checking for invalid arguments
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

    int* words_count;
    Dictnode* dictionary = make_dict(dictionary_path, max_word_size, &words_count);
    int* map_sizes = NULL;
    Bitmaps maps = make_maps(dictionary, max_word_size, words_count, &map_sizes);

    int wordnode_count = 0;
    Wordnode words = map_words(crossword, crossword_size, &wordnode_count);

    if (check_mode) {
    //     int ret_check = check_crossword(crossword, crossword_size, words, wordnode_count, dictionary, maps, map_sizes);
    //     if (draw_mode) draw_crossword(crossword, crossword_size);
    //     return ret_check;
    }
    //print_dict(dictionary, max_word_size);
    solve_crossword(crossword, dictionary, words, wordnode_count, maps, map_sizes);
    if (draw_mode) draw_crossword(crossword, crossword_size);
    draw_crossword(crossword, crossword_size);
    free_dict(dictionary, max_word_size);
    printf("dict: %s cross: %s max: %d\n", dictionary_path, crossword_path, max_word_size);
    return 0;
}