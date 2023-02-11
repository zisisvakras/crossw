#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "crossu.h"
#include "extratypes.h"
#include "extrafuns.h"

extern int errno;

/**
 * This function reads the crossword file and produces the grid based on the data.
 * It also returns the maximum word size on the grid and the size of the grid.
 */
void init_crossword(char* crossword_path, char*** crossword_ret, int* crossword_size_ret, int* max_word_size_ret) {
    char** crossword = NULL;
    int crossword_size = 0;
    int max_word_size = 0;

    FILE* crossword_file = fopen(crossword_path, "r");
    if (crossword_file == NULL) /* File error handling */
        error("Error while handling crossword", errno);

    if (fscanf(crossword_file, "%d", &crossword_size) != 1) /* Check failed size scan */
        error("Error while reading size of crossword", errno);
    if (crossword_size <= 0) {
        fprintf(stderr, "Invalid grid size of %d cannot proceed\n", crossword_size);
        exit(1);
    }
    
    /* Allocating memory for the grid */
    crossword = malloc(crossword_size * sizeof(char*));
    mallerr(crossword, errno);
    crossword[0] = malloc(crossword_size * crossword_size * sizeof(char));
    mallerr(crossword[0], errno);
    /* Setting every character with '-' to begin */
    memset(crossword[0], '-', crossword_size * crossword_size * sizeof(char));
    /* Fixing the pointers in the right spot */
    for (int i = 0 ; i < crossword_size - 1 ; ++i) {
        crossword[i + 1] = crossword[i] + crossword_size;
    }

    /* Reading the black tiles */
    int x, y;
    while (fscanf(crossword_file, "%d %d", &x, &y) == 2) {
        crossword[x - 1][y - 1] = '#'; /* Offset by 1 since data on file is based 1 */
    }

    /* Biggest word finder */
    for (int i = 0 ; i < crossword_size ; i++) {
        int len_row = 0, len_col = 0;
        for (int j = 0 ; j < crossword_size ; j++) {
            /* Row section */
            if (crossword[i][j] == '#') {
                if (len_row > max_word_size) max_word_size = len_row;
                len_row = 0;
            }
            if (crossword[i][j] == '-') len_row++;
            /* Column section */
            if (crossword[j][i] == '#') {
                if (len_col > max_word_size) max_word_size = len_col;
                len_col = 0;
            }
            if (crossword[j][i] == '-') len_col++;
        }
        if (len_row > max_word_size) max_word_size = len_row;
        if (len_col > max_word_size) max_word_size = len_col;
    }

    /* Returning the values */
    *crossword_ret = crossword;
    *crossword_size_ret = crossword_size;
    *max_word_size_ret = max_word_size;
    /* Cleanup */
    fclose(crossword_file);
}

/* Drawing the crossword as it is depicted in the assignment */
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
    mallerr(buffer, errno);

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
        /* Copying array with 1s */
        memcpy(map->array, maps[word_size - 1][word_size]->array, map->size * sizeof(int));
        /* Creating the appropriate map for the word given */
        for (int i = 0 ; buffer[i] ; ++i) {
            join_map(map, maps[word_size - 1][i] + (buffer[i] - 'a'));
        }
        /* If map has 0 bits that means word is not in dict */
        if (sum_bit(map) == 0) {
            fprintf(stderr, "Word \"%s\" not in dictionary\n", buffer);
            exit(1);
        }
        /* For vertical words we need to check if they match the horizontals already placed */
        if (words[index]->orientation) {
            int beg = words[index]->begin;
            for (int i = beg ; i <= words[index]->end ; ++i) {
                int ch = crossword[i][words[index]->constant];
                if (ch != '-') join_map(map, maps[word_size - 1][i - beg] + (ch - 'a'));
                if (sum_bit(map) == 0) {
                    fprintf(stderr, "Word \"%s\" cannot be placed\n", buffer);
                    exit(1);
                }
            }
        }
        /* Write word in crossword */
        write_word(crossword, words[index], buffer);
        index++;
    }
    if (index != wordnode_count) {
        fprintf(stderr, "Not enough words\n");
        exit(1);
    }
    free(buffer);
}

//TODO CHECK MAPS FOR SHIFT (MAYBE USE UNSIGNED)
//TODO ΜΑΚΕ AN ARRAY SIZE WORDNODE_COUNT THAT HOLDS ALL WORD POINTERS THAT NEED UPDATE
void solve_crossword(char*** crossword, int crossword_size, Dictionary* bigdict, Word** words, int wordnode_count, Map*** bitmaps) {
    /* Initilizing the crossword stack */
    char*** crosswords = init_crosswords(*crossword, crossword_size, wordnode_count);
    int index = 0, backtrack = 0;
    prop_word(words, wordnode_count, index);
    while (index < wordnode_count) {
        /* Find word in bigdict */
        char* word_found = NULL;
        if (backtrack || (word_found = find_word(bigdict[words[index]->size - 1], words[index])) == NULL) {
            if (index == 0) { /* Cannot backtrack from zero */
                fprintf(stderr, "Couldn\'t solve crossword ;-;\n");
                exit(1);
            }
            /* Fixing back all maps that got ruined from the word put */
            for (int i = 0 ; words[index - 1]->insecs[i].word ; ++i) {
                Word* word = words[index - 1]->insecs[i].word;
                if (word->in_use == 0) {
                    update_map(crosswords[index - 1], word, bitmaps);
                    sum_bit(word->map);
                }
            }
            words[index - 1]->in_use = 0;
            --index;
            backtrack = 0;
            continue;
        }
        /* Copying previous state before we write anything */
        memcpy(crosswords[index + 1][0], crosswords[index][0], crossword_size * crossword_size * sizeof(char));
        /* Write word found in copy state */
        write_word(crosswords[index + 1], words[index], word_found);
        /* Label word used */
        words[index]->in_use = 1;
        /* For every intersection in word update its map with the changed letter */
        for (int i = 0 ; words[index]->insecs[i].word ; ++i) {
            Intersection insec = words[index]->insecs[i];
            Word* word = insec.word;
            if (word->in_use == 0) {
                join_map(word->map, &bitmaps[word->size - 1][insec.pos][crosswords[index + 1][insec.x][insec.y] - 'a']);
                /* If some map turns out to be 0 do early backtrack (pruning the domain) */
                if (sum_bit(word->map) == 0) {
                    backtrack = 1;
                    break;
                }
            }
        }
        ++index;
        if (backtrack == 0) {
            if (index == wordnode_count) break;
            prop_word(words, wordnode_count, index);
        }
    }
    *crossword = crosswords[wordnode_count];
}

char*** init_crosswords(char** crossword, int crossword_size, int wordnode_count) {
    /* Crosswords initialization */
    char*** crosswords = malloc((wordnode_count + 1) * sizeof(char**));
    mallerr(crosswords, errno);
    for (int i = 0 ; i <= wordnode_count ; ++i) {
        crosswords[i] = malloc(crossword_size * sizeof(char*));
        mallerr(crosswords[i], errno);
        /* Allocating big memory block */
        crosswords[i][0] = malloc(crossword_size * crossword_size * sizeof(char));
        mallerr(crosswords[i][0], errno);
        /* Setting the pointers in the correct spot */
        for (int j = 0 ; j < crossword_size - 1 ; ++j) {
            crosswords[i][j + 1] = crosswords[i][j] + crossword_size;
        }
    }
    /* Initial copy */
    memcpy(crosswords[0][0], crossword[0], crossword_size * crossword_size * sizeof(char));
    return crosswords;
}
