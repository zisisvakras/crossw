#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "extratypes.h"
#include "extrafuns.h"

extern int errno;

void write_word(char** crossword, Word* node, char* word) {
    if (node->orientation) { /* Vertical */
        for (int i = node->begin, j = 0 ; i <= node->end ; i++, j++) {
            crossword[i][node->constant] = word[j];
        }
    }
    else { /* Horizontal */
        for (int i = node->begin, j = 0 ; i <= node->end ; i++, j++) {
            crossword[node->constant][i] = word[j];
        }
    }
}

//TODO add a second words array thats in the correct order for print and -check
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

int find_wordnode_count(char** crossword, int crossword_size) {
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
    return count;
}

//TODO optimize //TODO add mallerr to everything
Word** map_words(char** crossword, int crossword_size, int count, int** multi) {
    Word** words = malloc(count * sizeof(Word*));
    mallerr(words, errno);
    for (int i = 0 ; i < count ; ++i) {
        words[i] = malloc(sizeof(Word));
        mallerr(words[i], errno);
        memset(words[i], 0, sizeof(Word)); /* Setting most default values to 0 */
    }

    int hor_size = 0, ver_size = 0;
    int begin_h = 0, begin_v = 0, index = 0;
    for (int i = 0 ; i < crossword_size ; i++) {
        for (int j = 0 ; j < crossword_size ; j++) {
            if (crossword[i][j] != '#') {
                if (hor_size == 0) begin_h = j;
                hor_size++;
            }
            if (crossword[i][j] == '#') {
                if (hor_size > 1) {
                    *words[index++] = (Word) {
                        .orientation = 0,
                        .constant = i,
                        .begin = begin_h,
                        .end = j - 1,
                        .size = j - begin_h
                    };
                }
                hor_size = 0;
            }
            if (crossword[j][i] != '#') {
                if (ver_size == 0) begin_v = j;
                ver_size++;
            }
            if (crossword[j][i] == '#') {
                if (ver_size > 1) {
                    *words[index++] = (Word) {
                        .orientation = 1,
                        .constant = i,
                        .begin = begin_v,
                        .end = j - 1,
                        .size = j - begin_v
                    };
                }
                ver_size = 0;
            }
        }
        if (hor_size > 1) {
            *words[index++] = (Word) {
                .orientation = 0,
                .constant = i,
                .begin = begin_h,
                .end = crossword_size - 1,
                .size = crossword_size - begin_h
            };
        }
        if (ver_size > 1) {
            *words[index++] = (Word) {
                .orientation = 1,
                .constant = i,
                .begin = begin_v,
                .end = crossword_size - 1,
                .size = crossword_size - begin_v
            };
        }
        hor_size = 0;
        ver_size = 0;
    }
    for (int i = 0 ; i < count ; ++i) {
        Intersection* buf_insecs = calloc(count, sizeof(Intersection)); //TODO add checks
        int buf_insecc = 0;
        if (words[i]->orientation) {
            for (int j = words[i]->begin ; j <= words[i]->end ; ++j) {
                int found = 0;
                if (words[i]->constant != 0) {
                    char ch = crossword[j][words[i]->constant - 1];
                    if (ch != '#') {
                        for (int k = 0 ; k < count ; ++k) {
                            if (words[k]->orientation) continue;
                            int cord = words[i]->constant - 1;
                            if (words[k]->constant == j && words[k]->begin <= cord && cord <= words[k]->end) {
                                buf_insecs[buf_insecc++] = (Intersection) {
                                    .word = &(*words[k]),
                                    .x = j,
                                    .y = words[i]->constant,
                                    .pos = words[i]->constant - words[k]->begin
                                };
                                ++multi[words[i]->size - 1][j - words[i]->begin];
                                found = 1;
                                break;
                            }
                        }
                    }
                }
                if (!found && words[i]->constant != crossword_size - 1) {
                    char ch = crossword[j][words[i]->constant + 1];
                    if (ch != '#') {
                        for (int k = 0 ; k < count ; ++k) {
                            if (words[k]->orientation) continue;
                            int cord = words[i]->constant + 1;
                            if (words[k]->constant == j && words[k]->begin <= cord && cord <= words[k]->end) {
                                buf_insecs[buf_insecc++] = (Intersection) {
                                    .word = &(*words[k]),
                                    .x = j,
                                    .y = words[i]->constant,
                                    .pos = words[i]->constant - words[k]->begin
                                };
                                ++multi[words[i]->size - 1][j - words[i]->begin];
                                break;
                            }
                        }
                    }
                }
            }
        }
        else {
            for (int j = words[i]->begin ; j <= words[i]->end ; ++j) {
                int found = 0;
                if (words[i]->constant != 0) {
                    char ch = crossword[words[i]->constant - 1][j];
                    if (ch != '#') {
                        for (int k = 0 ; k < count ; ++k) {
                            if (!words[k]->orientation) continue;
                            int cord = words[i]->constant - 1;
                            if (words[k]->constant == j && words[k]->begin <= cord && cord <= words[k]->end) {
                                buf_insecs[buf_insecc++] = (Intersection) {
                                    .word = &(*words[k]),
                                    .x = words[i]->constant,
                                    .y = j,
                                    .pos = words[i]->constant - words[k]->begin
                                };
                                ++multi[words[i]->size - 1][j - words[i]->begin];
                                found = 1;
                                break;
                            }
                        }
                    }
                }
                if (!found && words[i]->constant != crossword_size - 1) {
                    char ch = crossword[words[i]->constant + 1][j];
                    if (ch != '#') {
                        // Check which word has letter
                        for (int k = 0 ; k < count ; ++k) {
                            if (!words[k]->orientation) continue;
                            int cord = words[i]->constant + 1;
                            if (words[k]->constant == j && words[k]->begin <= cord && cord <= words[k]->end) {
                               buf_insecs[buf_insecc++] = (Intersection) {
                                    .word = &(*words[k]),
                                    .x = words[i]->constant,
                                    .y = j,
                                    .pos = words[i]->constant - words[k]->begin
                                };
                                ++multi[words[i]->size - 1][j - words[i]->begin];
                                break;
                            }
                        }
                    }
                }
            }
        }
        words[i]->insecs = malloc((buf_insecc + 1) * sizeof(Intersection));
        memcpy(words[i]->insecs, buf_insecs, (buf_insecc + 1) * sizeof(Intersection));
        free(buf_insecs);
    }
    return words;
}

void prop_word(Word** words, int wordnode_count, int last) { //2nd criteria insecs
    int index = last;
    int min = words[index]->map->sum;
    for (int i = index + 1 ; i < wordnode_count ; ++i) {
        int temp = words[i]->map->sum;
        if (temp < min) {
            min = temp;
            index = i;
        }
    }
    Word* temp = words[last];
    words[last] = words[index];
    words[index] = temp;
}