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
/**
 * @details debug tool
*/
void print_words(Wordnode words, int wordnode_count) {
    for (int i = 0 ; i < wordnode_count ; ++i) {
        int c[4] = {
            words[i].orientation,
            words[i].constant,
            words[i].begin,
            words[i].end
        };
        printf("%d. %s: %d begin: %d end: %d", i + 1, c[0] ? "col" : "row", c[1], c[2], c[3]);
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
    int hor_size = 0, ver_size = 0, count = 0;
    for (int i = 0 ; i < crossword_size ; i++) {
        for (int j = 0 ; j < crossword_size ; j++) {
            if (crossword[i][j] != '#') hor_size++;
            if (crossword[i][j] == '#') {
                if (hor_size > 1) count++;
                hor_size = 0;
            }
            if (crossword[j][i] != '#') ver_size++;
            if (crossword[j][i] == '#') {
                if (ver_size > 1) count++;
                ver_size = 0;
            }
        }
        if (hor_size > 1) count++;
        if (ver_size > 1) count++;
        hor_size = 0;
        ver_size = 0;
    }
    *wordnode_count = count;

    Wordnode words = malloc(count * sizeof(Word));
    if (words == NULL) /* Malloc error handling */
        error("Error while allocating memory", errno);

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
                    words[index++] = (Word) {
                        .orientation = 0,
                        .constant = i,
                        .begin = begin_hor,
                        .end = j - 1
                    };
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
                    words[index++] = (Word) {
                        .orientation = 1,
                        .constant = i,
                        .begin = begin_ver,
                        .end = j - 1
                    };
                }
                ver_size = 0;
            }
        }
        if (hor_size > 1) {
            words[index++] = (Word) {
                .orientation = 0,
                .constant = i,
                .begin = begin_hor,
                .end = crossword_size - 1
            };
        }
        if (ver_size > 1) {
            words[index++] = (Word) {
                .orientation = 1,
                .constant = i,
                .begin = begin_ver,
                .end = crossword_size - 1
            };
        }
        hor_size = 0;
        ver_size = 0;
    }
    return words;
}

void prop_word(Action action, Wordnode words, int last, char** crossword, Bitmaps maps, int* map_sizes) {
    if (!last) return;
    int index = 0;
    int min = sum_bit(action.map[0], map_sizes[words[0].end - words[0].begin]);
    for (int i = 1 ; i <= last ; ++i) {
        int* map = action.map[i];
        Word word = words[i];
        int temp = sum_bit(map, map_sizes[word.end - word.begin]);
        if (temp < min) {
            min = temp;
            index = i;
        }
    }
    Word temp = words[last];
    words[last] = words[index];
    words[index] = temp;
    int* temp_m = action.map[last];
    action.map[last] = action.map[index];
    action.map[index] = temp_m;
}