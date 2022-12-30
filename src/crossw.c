#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "dict.h"
#include "crossutil.h"

extern int errno;

//TODO make dictionary also go backwards
int main(int argc, char** argv) {

    //TODO better checking for invalid arguments
    if (argc < 2) { /* Argument error handling */
        fprintf(stderr, "Not enough arguments\n");
        return -1;
    }

    char* dictionary_path = "Words.txt";
    char* crossword_path = argv[1];
    char* arg;
    //int check_mode = 0;
    //int draw_mode = 0;
    while (--argc) {
        if ((arg = argv[argc])) {
            if (!strcmp(arg, "-dict")) {
                if(argv[argc + 1] == NULL) { /* Argument error handling */
                    fprintf(stderr, "Could not find dictionary\n");
                    return -1;
                }
                dictionary_path = argv[argc + 1];
            }
            // if (!strcmp(arg, "-check")) {
            //     check_mode = 1;
            // }
            // if (!strcmp(arg, "-draw")) {
            //     draw_mode = 1;
            // }
        }
    } 
    FILE* crossword_file = fopen(crossword_path, "r");
    if (crossword_file == NULL) { /* File error handling */
        fprintf(stderr, "Error while handling dictionary: %s", strerror(errno));
        return errno;
    }

    int crossword_size = 0;
    if (fscanf(crossword_file, "%d", &crossword_size) != 1) {
        fprintf(stderr, "Error while reading size of crossword"); /* If scan didn't read 1 integer throw error */
        return errno;
    }
    
    //TODO check null pointer
    char** crossword = malloc(crossword_size * sizeof(char*));
    for (int i = 0 ; i < crossword_size ; i++) {
        crossword[i] = malloc(crossword_size * sizeof(char));
        //TODO remove later
        for (int j = 0 ; j < crossword_size ; j++) {
            crossword[i][j] = '-';
        }
    }

    int x, y;
    while (fscanf(crossword_file, "%d %d", &x, &y) == 2) { /* While it continues to read black tiles */
        crossword[x - 1][y - 1] = '#';
    }
    /* Biggest word finder */
    int max_word_size = 0;
    for (int i = 0 ; i < crossword_size ; i++) {
        int len_row = 0;
        int len_col = 0;
        for (int j = 0 ; j < crossword_size ; j++) {
            if (crossword[i][j] == '#') {
                if (len_row > max_word_size) max_word_size = len_row;
                len_row = 0;
            }
            if (crossword[i][j] == '-') len_row++;
            if (crossword[j][i] == '#') {
                if (len_col > max_word_size) max_word_size = len_col;
                len_col = 0;
            }
            if (crossword[j][i] == '-') len_col++;
        }
        if (len_row > max_word_size) max_word_size = len_row;
        if (len_col > max_word_size) max_word_size = len_col;
    }

    Dictnode* dictionary = make_dict(dictionary_path, max_word_size);
    if (dictionary == NULL) { /* Check dict.c for errors */
        return errno;
    }
    //print_dict(dictionary, max_word_size);
    //draw_crossword(crossword, crossword_size);

    // for (int i = 0 ; i < 1000000 ; i++) {
    //     find_word(dictionary, "zygote"); // last 6 letter word
    // }
    /* Closing files and deallocating memory */
    fclose(crossword_file);
    free_dict(dictionary, max_word_size);
    printf("dict: %s cross: %s max: %d\n", dictionary_path, crossword_path, max_word_size);
    return 0;
}