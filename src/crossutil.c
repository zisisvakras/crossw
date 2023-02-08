#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "extratypes.h"
#include "extrafuns.h"

extern int errno;

void init_crossword(char* crossword_path, char*** crossword, int* crossword_size, int* max_word_size) {
    
    FILE* crossword_file = fopen(crossword_path, "r");

    if (crossword_file == NULL) /* File error handling */
        error("Error while handling crossword", errno);

    if (fscanf(crossword_file, "%d", crossword_size) != 1) /* Check failed size scan */
        error("Error while reading size of crossword", errno);
    
    (*crossword) = malloc((*crossword_size) * sizeof(char*));
    if ((*crossword) == NULL) /* Malloc error handling */
        error("Error while allocating memory", errno);

    (*crossword)[0] = malloc((*crossword_size) * (*crossword_size) * sizeof(char));
    if ((*crossword)[0] == NULL) /* Malloc error handling */
        error("Error while allocating memory", errno);
    memset((*crossword)[0], '-', (*crossword_size) * (*crossword_size) * sizeof(char));

    for (int i = 1, j = (*crossword_size) ; i < (*crossword_size) ; ++i, j+=(*crossword_size)) {
        (*crossword)[i] = (*crossword)[0] + j;
    }

    /* Reading the black tiles */
    int x, y;
    while (fscanf(crossword_file, "%d %d", &x, &y) == 2) {
        (*crossword)[x - 1][y - 1] = '#';
    }

    /* Biggest word finder */
    int max = 0;
    for (int i = 0 ; i < (*crossword_size) ; i++) {
        int len_row = 0, len_col = 0;
        for (int j = 0 ; j < (*crossword_size) ; j++) {
            /* Row section */
            if ((*crossword)[i][j] == '#') {
                if (len_row > max) max = len_row;
                len_row = 0;
            }
            if ((*crossword)[i][j] == '-') len_row++;
            /* Column section */
            if ((*crossword)[j][i] == '#') {
                if (len_col > max) max = len_col;
                len_col = 0;
            }
            if ((*crossword)[j][i] == '-') len_col++;
        }
        if (len_row > max) max = len_row;
        if (len_col > max) max = len_col;
    }
    *max_word_size = max;

    fclose(crossword_file);
}

void draw_crossword(char** crossword, int crossword_size) {
    for (int i = 0 ; i < crossword_size ; i++) {
        for (int j = 0 ; j < crossword_size ; j++) {
            putchar(crossword[i][j] == '#' ? '#' : ' ');
            putchar(crossword[i][j]);
            putchar(crossword[i][j] == '#' ? '#' : ' ');
        }
        putchar('\n');
    }
}

void check_crossword(char** crossword, Word** words, Map*** maps, int wordnode_count) {
    char* buffer = malloc(sizeof(char) * 81);
    if (buffer == NULL) /* Malloc error handling */
        error("Error while allocating memory", errno);

    int index = 0;
    while (fscanf(stdin, "%80s", buffer) == 1) {
        if (index == wordnode_count) {
            fprintf(stderr, "More words than needed\n");
            exit(1);
        }
        int word_size = strlen(buffer);
        if (word_size != words[index]->size) {
            fprintf(stderr, "Word \"%s\" cannot be placed\n", buffer);
            exit(1);
        }
        Map* map = malloc(sizeof(Map));
        map->size = maps[word_size - 1][word_size]->size;
        map->array = malloc(map->size * sizeof(int));
        memcpy(map->array, maps[word_size - 1][word_size]->array, map->size * sizeof(int));
        for (int i = 0 ; buffer[i] ; ++i) {
            join_map(map, maps[word_size - 1][i] + (buffer[i] - 'a'));
        }
        if (sum_bit(map) == 0) {
            fprintf(stderr, "Word \"%s\" not in dictionary\n", buffer);
            exit(1);
        }
        if (words[index]->orientation) {
            int beg = words[index]->begin;
            for (int i = beg ; i <= words[index]->end ; ++i) {
                int ch = crossword[i][words[index]->constant];
                if (ch != '-') join_map(map, maps[word_size - 1][i - beg] + (ch - 'a'));
            }
        }
        else {
            int beg = words[index]->begin;
            for (int i = beg ; i <= words[index]->end ; ++i) {
                int ch = crossword[words[index]->constant][i];
                if (ch != '-') join_map(map, maps[word_size - 1][i - beg] + (ch - 'a'));
            }
        }
        if (sum_bit(map) == 0) {
            fprintf(stderr, "Word \"%s\" cannot be placed\n", buffer);
            exit(1);
        }
        write_word(crossword, words[index], buffer);
        index++;
    }
    if (index != wordnode_count) {
        fprintf(stderr, "Not enough words\n");
        exit(1);
    }
}

void solve_crossword(char*** crossword, int crossword_size, Dictionary* bigdict, Word** words, //TODO CHECK MAPS FOR SHIFT (MAYBE USE UNSIGNED)
                     int wordnode_count, Map*** bitmaps) {
                        //TODO ΜΑΚΕ AN ARRAY SIZE WORDNODE_COUNT THAT HOLDS ALL WORD POINTERS THAT NEED UPDATE
    char*** crosswords = init_crosswords(*crossword, crossword_size, wordnode_count);
    int index = 0; //TODO maybe add sum_bit inside update_map
    prop_word(words, wordnode_count, index);
    //int count = 0;
    while (index < wordnode_count) {
        /* Find word in bigdict */
        char* word_found = find_word(bigdict[words[index]->size - 1], words[index]);
        if (word_found == NULL) {
            if (index == 0) {
                fprintf(stderr, "what happend\n");
                exit(1);
            }
            backtrack:
            for (int i = 0 ; words[index - 1]->insecs[i].word ; ++i) {
                Word* word = words[index - 1]->insecs[i].word;
                if (word->in_use == 0) {
                    update_map(crosswords[index - 1], word, bitmaps);
                    sum_bit(word->map);
                }
            }
            words[index - 1]->in_use = 0;
            --index;
            continue;
        }
        memcpy(crosswords[index + 1][0], crosswords[index][0], crossword_size * crossword_size * sizeof(char));
        write_word(crosswords[index + 1], words[index], word_found);
        words[index]->in_use = 1;
        //TODO prune without goto
        for (int i = 0 ; words[index]->insecs[i].word ; ++i) {
            Intersection insec = words[index]->insecs[i];
            Word* word = insec.word;
            if (word->in_use == 0) {
                join_map(word->map, &bitmaps[word->size - 1][insec.pos][crosswords[index + 1][insec.x][insec.y] - 'a']);
                if (sum_bit(word->map) == 0) {
                    ++index;
                    goto backtrack;
                }
            }
        }
        // ++count;
        // if (count == 100000) {
        //     count = 0;
        //     putchar('\n');
        //     draw_crossword(crosswords[index], crossword_size);
        //     putchar('\n');
        // }
        ++index;
        if (index == wordnode_count) break;
        prop_word(words, wordnode_count, index);
    }
    *crossword = crosswords[wordnode_count];
}

char*** init_crosswords(char** crossword, int crossword_size, int wordnode_count) {
    /* Crosswords initialization */
    char*** crosswords = malloc((wordnode_count + 1) * sizeof(char**));
    if (crosswords == NULL) /* Malloc error handling */
        error("Error while allocating memory", errno);
    for (int i = 0 ; i <= wordnode_count ; ++i) {
        crosswords[i] = malloc(crossword_size * sizeof(char*));
        if (crosswords[i] == NULL) /* Malloc error handling */
            error("Error while allocating memory", errno);
        crosswords[i][0] = malloc(crossword_size * crossword_size * sizeof(char));
        if (crosswords[i][0] == NULL) /* Malloc error handling */
            error("Error while allocating memory", errno);
        for (int j = 1, k = crossword_size ; j < crossword_size ; ++j, k+=crossword_size) {
            crosswords[i][j] = crosswords[i][0] + k;
        }
    }
    /* Initial copy */
    memcpy(crosswords[0][0], crossword[0], crossword_size * crossword_size * sizeof(char));
    return crosswords;
}
