#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "extratypes.h"
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
void print_words(Wordnode words, int wordnode_count) {
    for (int i = 0 ; i < wordnode_count ; ++i) {
        if (words[i].orientation) {
            printf("%d. col: %d begin: %d end: %d", i + 1, words[i].constant, words[i].begin, words[i].end);
        }
        else {
            printf("%d. row: %d begin: %d end: %d", i + 1, words[i].constant, words[i].begin, words[i].end);
        }
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

Wordnode map_words(char** crossword, int crossword_size, int* wordnode_count) {
    int hor_size = 0, ver_size = 0, ver_count = 0, hor_count = 0;
    for (int i = 0 ; i < crossword_size ; i++) {
        for (int j = 0 ; j < crossword_size ; j++) {
            if (crossword[i][j] != '#') hor_size++;
            if (crossword[i][j] == '#') {
                if (hor_size > 1) hor_count++;
                hor_size = 0;
            }
            if (crossword[j][i] != '#') ver_size++;
            if (crossword[j][i] == '#') {
                if (ver_size > 1) ver_count++;
                ver_size = 0;
            }
        }
        if (hor_size > 1) hor_count++;
        if (ver_size > 1) ver_count++;
        hor_size = 0;
        ver_size = 0;
    }
    *wordnode_count = hor_count + ver_count;

    Wordnode words = malloc((*wordnode_count) * sizeof(Word));

    if (words == NULL) { /* Malloc error handling */
        fprintf(stderr, "Error while allocating memory: %s", strerror(errno));
        return NULL;
    }
    int begin_hor, begin_ver, index = 0;
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
                    words[index++] = (Word){0, i, begin_hor, j - 1};
                    // words[index++] = (Word){
                    //     .orientation = 0,
                    //     .constant = i,
                    //     .begin = begin_hor,
                    //     .end = j - 1
                    // };
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
                    words[index++] = (Word){1, i, begin_ver, j - 1};
                }
                ver_size = 0;
            }
        }
        if (hor_size > 1) {
            words[index++] = (Word){0, i, begin_hor, crossword_size - 1};
        }
        if (ver_size > 1) {
            words[index++] = (Word){1, i, begin_ver, crossword_size - 1};
        }
        hor_size = 0;
        ver_size = 0;
    }
    return words;
}

void prop_word(Wordnode words, int last, char** crossword, Bitmaps maps, int* map_sizes) {
    if (!last) return;
    int index = 0;
    char* filter = create_filter(crossword, words[0]);
    int* map = create_map(maps, map_sizes, filter);
    int min = sum_bit(map, map_sizes[strlen(filter) - 1]);
    free(map);
    free(filter);
    for (int i = 1 ; i <= last ; ++i) {
        filter = create_filter(crossword, words[i]);
        map = create_map(maps, map_sizes, filter);
        int temp = sum_bit(map, map_sizes[strlen(filter) - 1]);
        if (temp < min) {
            min = temp;
            index = i;
        }
        free(map);
        free(filter);
    }
    Word temp = words[last];
    words[last] = words[index];
    words[index] = temp;
}