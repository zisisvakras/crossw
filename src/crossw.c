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

    /* Multipliers */
    int** multi = malloc(max_word_size * sizeof(int*));
    mallerr(multi, errno);
    for (int i = 0 ; i < max_word_size ; ++i) {
        multi[i] = calloc((i + 1), sizeof(int));
        mallerr(multi[i], errno);
    }
    
    /* Map the crossword */
    int wordnode_count = find_wordnode_count(crossword, crossword_size);
    Word** words = map_words(crossword, crossword_size, wordnode_count, multi);

    /* Initialize dictionaries */
    int* words_count = NULL;
    Dictionary* bigdict = init_dictionary(dictionary_path, max_word_size, &words_count, multi);

    /* Initialize maps */
    Map*** maps = init_maps(bigdict, max_word_size, words_count);

    
    /* Initializing maps for all words */
    for (int i = 0 ; i < wordnode_count ; ++i) {
        Map* src = maps[words[i]->size - 1][words[i]->size];
        words[i]->map = malloc(sizeof(Map));
        words[i]->map->size = src->size;
        words[i]->map->array = malloc(src->size * sizeof(int));
        memcpy(words[i]->map->array, src->array, src->size * sizeof(int));
        sum_bit(words[i]->map);
    }

    if (check_mode) {
        Word** ord_words = malloc(wordnode_count * sizeof(Word*));
        int ord_i = 0;
        for (int i = 0 ; i < wordnode_count ; ++i) {
            if (!(words[i]->orientation)) {
                ord_words[ord_i++] = words[i];
            }
        }
        for (int i = 0 ; i < wordnode_count ; ++i) {
            if (words[i]->orientation) {
                ord_words[ord_i++] = words[i];
            }
        }
        check_crossword(crossword, ord_words, maps, wordnode_count);
        if (draw_mode) draw_crossword(crossword, crossword_size);
        printf("dict: %s cross: %s max: %d words: %d\n", dictionary_path, crossword_path, max_word_size, wordnode_count);
        return 0;
    }
    //print_dict(bigdict, max_word_size);
    //print_words(words, wordnode_count);
    solve_crossword(&crossword, crossword_size, bigdict, words, wordnode_count, maps);
    if (draw_mode) draw_crossword(crossword, crossword_size);
    else print_solution(crossword, crossword_size);
    free_dictionary(bigdict, max_word_size, words_count);
    return 0;
}