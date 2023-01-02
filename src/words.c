#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "extrafuns.h"

extern int errno;

void write_word(char** crossword, Word node, char* word) {
    if (node.orientation) { /* Vertical */
        for (int i = node.begin, j = 0 ; i <= node.end ; i++, j++) {
            crossword[i][node.constant] = word[j];
        }
    }
    else { /* Horizontal */
        for (int i = node.begin, j = 0 ; i <= node.end ; i++, j++) {
            crossword[node.constant][i] = word[j];
        }
    }
}

void delete_word(char** crossword, Word node, char* word) {
    if (node.orientation) { /* Vertical */
        for (int i = node.begin, j = 0 ; i <= node.end ; i++, j++) {
            if (word[j] != '?') crossword[i][node.constant] = '-';
        }
    }
    else { /* Horizontal */
        for (int i = node.begin, j = 0 ; i <= node.end ; i++, j++) {
            if (word[j] != '?') crossword[node.constant][i] = '-';
        }
    }
}

//TODO change to actual values
void print_words(Wordnode* words, int hor_count, int ver_count) {
    printf("\nHorizontal words:\n");
    for (int i = 0 ; i < hor_count ; i++) {
        printf("%d. row: %d begin: %d end: %d\n", i+1, words[0][i].constant, words[0][i].begin, words[0][i].end);
    }
    printf("\nVertical words:\n");
    for (int i = 0 ; i < ver_count ; i++) {
        printf("%d. col: %d begin: %d end: %d\n", i+1, words[1][i].constant, words[1][i].begin, words[1][i].end);
    }
}

char* word_written(char* word, char* filter) {
    int size = strlen(word);
    char* written = malloc((size + 1) * sizeof(char));
    for (int i = 0 ; i < size ; i++) {
        written[i] = filter[i] == '?' ? word[i] : '?';
    }
    written[size] = '\0';
    return written;
}

Wordnode* map_words(char** crossword, int crossword_size, int* hor_count, int* ver_count) {
    int hor_size = 0;
    int ver_size = 0;
    for (int i = 0 ; i < crossword_size ; i++) {
        for (int j = 0 ; j < crossword_size ; j++) {
            if (crossword[i][j] != '#') hor_size++;
            if (crossword[i][j] == '#') {
                if (hor_size > 1) (*hor_count)++;
                hor_size = 0;
            }
            if (crossword[j][i] != '#') ver_size++;
            if (crossword[j][i] == '#') {
                if (ver_size > 1) (*ver_count)++;
                ver_size = 0;
            }
        }
        if (hor_size > 1) (*hor_count)++;
        if (ver_size > 1) (*ver_count)++;
        hor_size = 0;
        ver_size = 0;
    }
    Wordnode* words = malloc(2 * sizeof(Wordnode));
    if (words == NULL) { /* Malloc error handling */
        fprintf(stderr, "Error while allocating memory: %s", strerror(errno));
        return NULL;
    }
    words[0] = malloc((*hor_count) * sizeof(Word));
    words[1] = malloc((*ver_count) * sizeof(Word));
    if (words[0] == NULL || words[1] == NULL) { /* Malloc error handling */
        fprintf(stderr, "Error while allocating memory: %s", strerror(errno));
        return NULL;
    }
    int begin_hor, begin_ver;
    int hor_index = 0, ver_index = 0;
    for (int i = 0 ; i < crossword_size ; i++) {
        for (int j = 0 ; j < crossword_size ; j++) {
            if (crossword[i][j] != '#') {
                if (hor_size == 0) {
                    begin_hor = j;
                }
                hor_size++;
            }
            if (crossword[i][j] == '#') {
                if (hor_size > 1) {  
                    words[0][hor_index].orientation = 0;
                    words[0][hor_index].constant = i;
                    words[0][hor_index].begin = begin_hor;
                    words[0][hor_index].end = j - 1;
                    hor_index++;
                }
                hor_size = 0;
            }
            if (crossword[j][i] != '#') {
                if (ver_size == 0) {
                    begin_ver = j;
                }
                ver_size++;
            }
            if (crossword[j][i] == '#') {
                if (ver_size > 1) {
                    words[1][hor_index].orientation = 1;
                    words[1][ver_index].constant = i;
                    words[1][ver_index].begin = begin_ver;
                    words[1][ver_index].end = j - 1;
                    ver_index++;
                }
                ver_size = 0;
            }
        }
        if (hor_size > 1) {
            words[0][hor_index].orientation = 0;
            words[0][hor_index].constant = i;
            words[0][hor_index].begin = begin_hor;
            words[0][hor_index].end = crossword_size - 1;
            hor_index++;
        }
        if (ver_size > 1) {
            words[1][hor_index].orientation = 1;
            words[1][ver_index].constant = i;
            words[1][ver_index].begin = begin_ver;
            words[1][ver_index].end = crossword_size - 1;
            ver_index++;
        }
        hor_size = 0;
        ver_size = 0;
    }
    return words;
}