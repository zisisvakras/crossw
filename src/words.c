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

/**
 * @details debug tool
*/
void print_words(Wordnode words, int wordnode_count) {
    for (int i = 0 ; i < wordnode_count ; ++i) {
        int c[5] = {
            words[i].orientation,
            words[i].constant,
            words[i].begin,
            words[i].end,
            words[i].size
        };
        printf("%d. %s: %d begin: %d end: %d size: %d\n", i + 1, c[0] ? "col" : "row", c[1], c[2], c[3], c[4]);
    }
}

/**
 * @details debug tool
*/
void print_words_val(char** crossword, Wordnode words, int wordnode_count) {
    for (int i = 0 ; i < wordnode_count ; ++i) {
        printf("%d. (%s)", i, words[i].orientation ? "ver" : "hor");
        if (words[i].orientation) {
            for (int j = words[i].begin ; j <= words[i].end ; ++j) {
                putchar(crossword[j][words[i].constant]);
            }
        }
        else {
            for (int j = words[i].begin ; j <= words[i].end ; ++j) {
                putchar(crossword[words[i].constant][j]);
            }
        }
        putchar('\n');
    }
}

void print_solution(char** crossword, int crossword_size) {
    int flag = 0;
    for (int i = 0 ; i < crossword_size ; i++) {
        for (int j = 0 ; j < crossword_size ; j++) {
            if (crossword[i][j] != '#') {
                if (!flag) {
                    if (j == crossword_size - 1) continue;
                    if (crossword[i][j + 1] != '#') flag = 1;
                    else continue;
                }
                putchar(crossword[i][j]);
            }
            if (crossword[i][j] == '#') {
                if (flag) putchar('\n');
                flag = 0;
            }
        }
        if (flag) putchar('\n');
        flag = 0;
    }
    for (int i = 0 ; i < crossword_size ; i++) {
        for (int j = 0 ; j < crossword_size ; j++) {
            if (crossword[j][i] != '#') {
                if (!flag) {
                    if (j == crossword_size - 1) continue;
                    if (crossword[j + 1][i] != '#') flag = 1;
                    else continue;
                }
                putchar(crossword[j][i]);
            }
            if (crossword[j][i] == '#') {
                if (flag) putchar('\n');
                flag = 0;
            }
        }
        if (flag) putchar('\n');
        flag = 0;
    }
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
                        .end = j - 1,
                        .size = j - begin_hor
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
                        .end = j - 1,
                        .size = j - begin_ver
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
                .end = crossword_size - 1,
                .size = crossword_size - begin_hor
            };
        }
        if (ver_size > 1) {
            words[index++] = (Word) {
                .orientation = 1,
                .constant = i,
                .begin = begin_ver,
                .end = crossword_size - 1,
                .size = crossword_size - begin_ver
            };
        }
        hor_size = 0;
        ver_size = 0;
    }
    return words;
}

void prop_word(State* states, int** maps, Wordnode words, int last, int* map_sizes, int wordnode_count) {
    if (last == wordnode_count - 1) return;
    int index = last;
    int min = sum_bit(maps[index], map_sizes[words[index].size - 1]);
    for (int i = index + 1 ; i < wordnode_count ; ++i) {
        int temp = sum_bit(maps[i], map_sizes[words[i].size - 1]);
        if (temp < min) {
            min = temp;
            index = i;
        }
    }
    Word temp = words[last];
    words[last] = words[index];
    words[index] = temp;
    for (int i = 0 ; i < wordnode_count ; ++i) {
        int* temp_m = states[i].map[last];
        states[i].map[last] = states[i].map[index];
        states[i].map[index] = temp_m;
    }
}