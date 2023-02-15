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

//TODO change all ++ to pre
void print_solution(char** crossword, Word** ord_words, int count) {
    for (int i = 0 ; i < count ; ++i) {
        int c = ord_words[i]->constant;
        int f = ord_words[i]->orientation;
        for (int j = ord_words[i]->begin ; j <= ord_words[i]->end ; ++j) {
            putchar(f ? crossword[j][c] : crossword[c][j]); /* Choose cell depending on orientation */
        }
        putchar('\n');
    }
}

/* Count words on the crossword */
int count_words_on_grid(char** crossword, int crossword_size) {
    int hor_size = 0, ver_size = 0, count = 0;
    for (int i = 0 ; i < crossword_size ; i++) {
        for (int j = 0 ; j < crossword_size ; j++) {
            /* Horizontal */
            if (crossword[i][j] != '#') hor_size++;
            if (crossword[i][j] == '#' || j == crossword_size - 1) {
                if (hor_size > 1) count++; /* Words with size 1 don't count */
                hor_size = 0;
            }
            /* Vertical */
            if (crossword[j][i] != '#') ver_size++;
            if (crossword[j][i] == '#' || j == crossword_size - 1) {
                if (ver_size > 1) count++;
                ver_size = 0;
            }
        }
    }
    return count;
}

Word** map_words_on_grid(char** crossword, int crossword_size, int count, int** multi) {
    /* Initializing grid_words */
    Word** grid_words = malloc(count * sizeof(Word*));
    mallerr(grid_words, errno);
    for (int i = 0 ; i < count ; ++i) {
        grid_words[i] = calloc(1, sizeof(Word)); /* Setting most default values to 0 */
        mallerr(grid_words[i], errno);
    }

    /* Mapping the grid */
    int hor_size = 0, ver_size = 0;
    int begin_h = 0, begin_v = 0, index = 0;
    for (int i = 0 ; i < crossword_size ; i++) {
        for (int j = 0 ; j < crossword_size ; j++) {
            if (crossword[i][j] != '#') {
                if (hor_size == 0) begin_h = j;
                hor_size++;
            }
            if (crossword[i][j] == '#' || j == crossword_size - 1) {
                int newj = ((crossword[i][j] != '#') ? j : (j - 1));
                if (hor_size > 1) {
                    *grid_words[index++] = (Word) {
                        .orientation = 0,
                        .constant = i,
                        .begin = begin_h,
                        .end = newj,
                        .size = newj - begin_h + 1
                    };
                }
                hor_size = 0;
            }
            if (crossword[j][i] != '#') {
                if (ver_size == 0) begin_v = j;
                ver_size++;
            }
            if (crossword[j][i] == '#' || j == crossword_size - 1) {
                int newj = ((crossword[j][i] != '#') ? j : (j - 1));
                if (ver_size > 1) {
                    *grid_words[index++] = (Word) {
                        .orientation = 1,
                        .constant = i,
                        .begin = begin_v,
                        .end = newj,
                        .size = newj - begin_v + 1
                    };
                }
                ver_size = 0;
            }
        }
    }

    /* Finding the intersections */
    //TODO remove extra NULL insecs
    Intersection* buf_insecs = malloc(count * sizeof(Intersection));
    mallerr(buf_insecs, errno);
    int buf_insecc = 0;
    for (int i = 0 ; i < count ; ++i) {
        /* Reseting buf_insecs */
        memset(buf_insecs, 0, count * sizeof(Intersection));
        buf_insecc = 0;
        for (int j = grid_words[i]->begin ; j <= grid_words[i]->end ; ++j) {
            for (int k = 0 ; k < count ; ++k) {
                if (grid_words[k]->orientation == grid_words[i]->orientation) continue;
                int cord = grid_words[i]->constant;
                if (grid_words[k]->constant == j && grid_words[k]->begin <= cord && cord <= grid_words[k]->end) {
                    buf_insecs[buf_insecc++] = (Intersection) {
                        .word = &(*grid_words[k]),
                        .x = grid_words[i]->orientation ? j : cord,
                        .y = grid_words[i]->orientation ? cord : j,
                        .pos = cord - grid_words[k]->begin,
                        .pos_l = j - grid_words[i]->begin
                    };
                    ++multi[grid_words[i]->size - 1][j - grid_words[i]->begin];
                    break;
                }
            }
        }
        grid_words[i]->insecs = malloc((buf_insecc + 1) * sizeof(Intersection));
        grid_words[i]->insecc = buf_insecc;
        memcpy(grid_words[i]->insecs, buf_insecs, (buf_insecc + 1) * sizeof(Intersection));
    }
    free(buf_insecs);
    return grid_words;
}

void prop_word(Word** words, int wordnode_count, int last) {
    int index = last;
    int min = words[index]->map->sum;
    int insec = words[index]->insecc;
    /**
     * Looping through all words to find the one with lowest possibilities
     * as a 2nd criteria we are also looking for the number of interesections
     * note for the 2nd one: this might not be needed we are not sure ¯\_(ツ)_/¯
     */
    for (int i = index + 1 ; i < wordnode_count ; ++i) {
        int temp = words[i]->map->sum;
        if (temp < min) {
            min = temp;
            insec = words[i]->insecc;
            index = i;
        }
        else if (temp == min && words[i]->insecc > insec) {
            min = temp;
            insec = words[i]->insecc;
            index = i;
        }
    }
    /* Swapping the pointers */
    Word* temp = words[last];
    words[last] = words[index];
    words[index] = temp;
}