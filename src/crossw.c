#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "extrafuns.h"

extern int errno;

//TODO better alphabet for sorted double linked list by total (freq) value of letters
//TODO make dictionary also go backwards maybe
int main(int argc, char** argv) {

    //TODO better checking for invalid arguments
    if (argc < 2) { /* Argument error handling */
        fprintf(stderr, "Not enough arguments\n");
        return -1;
    }

    char* dictionary_path = "Words.txt";
    char* crossword_path = argv[1];
    char* arg;
    int check_mode = 0;
    int draw_mode = 0;
    while (--argc) {
        if ((arg = argv[argc])) {
            if (!strcmp(arg, "-dict")) {
                if(argv[argc + 1] == NULL) { /* Argument error handling */
                    fprintf(stderr, "Could not find dictionary\n");
                    return -1;
                }
                dictionary_path = argv[argc + 1];
            }
            if (!strcmp(arg, "-check")) {
                check_mode = 1;
            }
            if (!strcmp(arg, "-draw")) {
                draw_mode = 1;
            }
        }
    }
    
    char** crossword;
    int max_word_size = 0, crossword_size = 0;
    if (init_crossword(crossword_path, &crossword, &crossword_size, &max_word_size)) return errno;

    Dictnode* dictionary = make_dict(dictionary_path, max_word_size);
    if (dictionary == NULL) { /* Check dict.c for errors */
        return errno;
    }
    
    int hor_count = 0;
    int ver_count = 0;
    Wordnode* words = map_words(crossword, crossword_size, &hor_count, &ver_count);

    if (check_mode) {
        int ret_check = check_crossword(crossword, crossword_size, words, hor_count, ver_count, dictionary);
        if (draw_mode) draw_crossword(crossword, crossword_size);
        return ret_check;
    }
    solve_crossword(crossword, crossword_size, dictionary, words, hor_count, ver_count);
    if (draw_mode) draw_crossword(crossword, crossword_size);
    draw_crossword(crossword, crossword_size);
    free_dict(dictionary, max_word_size);
    printf("dict: %s cross: %s max: %d\n", dictionary_path, crossword_path, max_word_size);
    return 0;
}