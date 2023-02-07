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
void print_words(Word* words, int wordnode_count) {
    for (int i = 0 ; i < wordnode_count ; ++i) {
        int c[5] = {
            words[i].orientation,
            words[i].constant,
            words[i].begin,
            words[i].end,
            words[i].size
        };
        printf("(%p) %s: %d begin: %d end: %d size: %d\n", words + i, c[0] ? "col" : "row", c[1], c[2], c[3], c[4]);
        print_map(words[i].map);
        /* Interesections */
        printf("insecs:\n");
        int j = -1;
        while(words[i].insecs[++j]) printf("%p\n", words[i].insecs[j]);
    }
}

/**
 * @details debug tool
*/
void print_words_val(char** crossword, Word* words, int wordnode_count) {
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

//TODO optimize
Word* map_words(char** crossword, int crossword_size, int* wordnode_count, Map*** bitmaps) {
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

    Word* words = malloc(count * sizeof(Word));
    if (words == NULL) /* Malloc error handling */
        error("Error while allocating memory", errno);

    int begin_h, begin_v, index = 0;
    for (int i = 0 ; i < crossword_size ; i++) {
        for (int j = 0 ; j < crossword_size ; j++) {
            if (crossword[i][j] != '#') {
                if (hor_size == 0) begin_h = j;
                hor_size++;
            }
            if (crossword[i][j] == '#') {
                if (hor_size > 1) {
                    words[index] = (Word) {
                        .in_use = 0,
                        .orientation = 0,
                        .constant = i,
                        .begin = begin_h,
                        .end = j - 1,
                        .size = j - begin_h,
                        .insecs = NULL
                    };
                    Map* src = bitmaps[words[index].size - 1][words[index].size];
                    words[index].map = malloc(sizeof(Map));
                    words[index].map->size = src->size;
                    words[index].map->array = malloc(src->size * sizeof(int));
                    memcpy(words[index].map->array, src->array, src->size * sizeof(int));
                    sum_bit(words[index].map);
                    ++index;
                }
                hor_size = 0;
            }
            if (crossword[j][i] != '#') {
                if (ver_size == 0) begin_v = j;
                ver_size++;
            }
            if (crossword[j][i] == '#') {
                if (ver_size > 1) {
                    words[index] = (Word) {
                        .in_use = 0,
                        .orientation = 1,
                        .constant = i,
                        .begin = begin_v,
                        .end = j - 1,
                        .size = j - begin_v,
                        .insecs = NULL
                    };
                    Map* src = bitmaps[words[index].size - 1][words[index].size];
                    words[index].map = malloc(sizeof(Map));
                    words[index].map->size = src->size;
                    words[index].map->array = malloc(src->size * sizeof(int));
                    memcpy(words[index].map->array, src->array, src->size * sizeof(int));
                    sum_bit(words[index].map);
                    ++index;
                }
                ver_size = 0;
            }
        }
        if (hor_size > 1) {
            words[index] = (Word) {
                .in_use = 0,
                .orientation = 0,
                .constant = i,
                .begin = begin_h,
                .end = crossword_size - 1,
                .size = crossword_size - begin_h,
                .insecs = NULL
            };
            Map* src = bitmaps[words[index].size - 1][words[index].size];
            words[index].map = malloc(sizeof(Map));
            words[index].map->size = src->size;
            words[index].map->array = malloc(src->size * sizeof(int));
            memcpy(words[index].map->array, src->array, src->size * sizeof(int));
            sum_bit(words[index].map);
            ++index;
        }
        if (ver_size > 1) {
            words[index] = (Word) {
                .in_use = 0,
                .orientation = 1,
                .constant = i,
                .begin = begin_v,
                .end = crossword_size - 1,
                .size = crossword_size - begin_v,
                .insecs = NULL
            };
            Map* src = bitmaps[words[index].size - 1][words[index].size];
            words[index].map = malloc(sizeof(Map));
            words[index].map->size = src->size;
            words[index].map->array = malloc(src->size * sizeof(int));
            memcpy(words[index].map->array, src->array, src->size * sizeof(int));
            sum_bit(words[index].map);
            ++index;
        }
        hor_size = 0;
        ver_size = 0;
    }
    for (int i = 0 ; i < (*wordnode_count) ; ++i) {
        Word** buf_insecs = calloc((*wordnode_count), sizeof(Word*)); //TODO add checks
        int buf_insecc = 0;
        if (words[i].orientation) {
            for (int j = words[i].begin ; j <= words[i].end ; ++j) {
                int found = 0;
                if (words[i].constant != 0) {
                    char ch = crossword[j][words[i].constant - 1];
                    if (ch != '#') {
                        for (int k = 0 ; k < (*wordnode_count) ; ++k) {
                            if (words[k].orientation) continue;
                            int cord = words[i].constant - 1;
                            if (words[k].constant == j && words[k].begin <= cord && cord <= words[k].end) {
                                buf_insecs[buf_insecc] = &words[k];
                                ++buf_insecc;
                                found = 1;
                                break;
                            }
                        }
                    }
                }
                if (!found && words[i].constant != crossword_size - 1) {
                    char ch = crossword[j][words[i].constant + 1];
                    if (ch != '#') {
                        for (int k = 0 ; k < (*wordnode_count) ; ++k) {
                            if (words[k].orientation) continue;
                            int cord = words[i].constant + 1;
                            if (words[k].constant == j && words[k].begin <= cord && cord <= words[k].end) {
                                buf_insecs[buf_insecc] = &words[k];
                                ++buf_insecc;
                                break;
                            }
                        }
                    }
                }
            }
        }
        else {
            for (int j = words[i].begin ; j <= words[i].end ; ++j) {
                int found = 0;
                if (words[i].constant != 0) {
                    char ch = crossword[words[i].constant - 1][j];
                    if (ch != '#') {
                        for (int k = 0 ; k < (*wordnode_count) ; ++k) {
                            if (!words[k].orientation) continue;
                            int cord = words[i].constant - 1;
                            if (words[k].constant == j && words[k].begin <= cord && cord <= words[k].end) {
                                buf_insecs[buf_insecc] = &words[k];
                                ++buf_insecc;
                                found = 1;
                                break;
                            }
                        }
                    }
                }
                if (!found && words[i].constant != crossword_size - 1) {
                    char ch = crossword[words[i].constant + 1][j];
                    if (ch != '#') {
                        // Check which word has letter
                        for (int k = 0 ; k < (*wordnode_count) ; ++k) {
                            if (!words[k].orientation) continue;
                            int cord = words[i].constant + 1;
                            if (words[k].constant == j && words[k].begin <= cord && cord <= words[k].end) {
                                buf_insecs[buf_insecc] = &words[k];
                                ++buf_insecc;
                                break;
                            }
                        }
                    }
                }
            }
        }
        words[i].insecs = malloc((buf_insecc + 1) * sizeof(Word*));
        memcpy(words[i].insecs, buf_insecs, (buf_insecc + 1) * sizeof(Word*));
        free(buf_insecs);
    }
    return words;
}

void prop_word(Word* words, int wordnode_count, int last) {
    DBGCHECK(last != wordnode_count - 1);
    int index = last;
    int min = words[index].map->sum;
    for (int i = index + 1 ; i < wordnode_count ; ++i) {
        int temp = words[i].map->sum;
        if (temp < min) {
            min = temp;
            index = i;
        }
    }
    Word temp = words[last];
    words[last] = words[index];
    words[index] = temp;
}