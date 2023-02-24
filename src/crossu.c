#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
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
    /* Setting every character with \0 to begin */
    crossword[0] = calloc(crossword_size * crossword_size, sizeof(char));
    mallerr(crossword[0], errno);
    /* Fixing the pointers in the right spot */
    for (int i = 0 ; i < crossword_size - 1 ; ++i) {
        crossword[i + 1] = crossword[i] + crossword_size;
    }

    /* Reading the black tiles */
    int x, y;
    while (fscanf(crossword_file, "%d %d", &x, &y) == 2) {
        /* Using bell to mark black tiles since # can be in words */
        crossword[x - 1][y - 1] = '\r'; /* Offset by 1 since data on file is based 1 */
    }

    /* Biggest word finder */
    for (int i = 0 ; i < crossword_size ; ++i) {
        int len_row = 0, len_col = 0;
        for (int j = 0 ; j < crossword_size ; ++j) {
            /* Row section */
            if (crossword[i][j] == '\r') {
                if (len_row > max_word_size) max_word_size = len_row;
                len_row = 0;
            }
            if (crossword[i][j] == '\0') ++len_row;
            /* Column section */
            if (crossword[j][i] == '\r') {
                if (len_col > max_word_size) max_word_size = len_col;
                len_col = 0;
            }
            if (crossword[j][i] == '\0') ++len_col;
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
    for (int i = 0 ; i < crossword_size ; ++i) {
        for (int j = 0 ; j < crossword_size ; ++j) {
            if (crossword[i][j] == '\r') {
                printf("###");
            } else {
                printf(" %c ", crossword[i][j]);
            }
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
        map->array = malloc(map->size * sizeof(long long));
        /* Copying array with 1s */
        memcpy(map->array, maps[word_size - 1][word_size]->array, map->size * sizeof(long long));
        /* Creating the appropriate map for the word given */
        for (int i = 0 ; buffer[i] ; ++i) {
            join_map(map, maps[word_size - 1][i] + buffer[i]);
        }
        /* If map has 0 bits that means word is not in dict */
        if (sum_bit(map) == 0) {
            fprintf(stderr, "Word \"%s\" not in dictionary\n", buffer);
            exit(1);
        }
        /* For vertical words we need to check if they match the horizontals already placed */
        if (words[index]->orientation == Vertical) {
            int beg = words[index]->begin;
            for (int i = beg ; i <= words[index]->end ; ++i) {
                int ch = crossword[i][words[index]->constant];
                if (ch != '\0') join_map(map, maps[word_size - 1][i - beg] + ch);
                if (sum_bit(map) == 0) {
                    fprintf(stderr, "Word \"%s\" cannot be placed\n", buffer);
                    exit(1);
                }
            }
        }
        /* Write word in crossword */
        write_word(crossword, words[index], buffer);
        ++index;
    }
    if (index != wordnode_count) {
        fprintf(stderr, "Not enough words\n");
        exit(1);
    }
    free(buffer);
}

//TODO CHECK MAPS FOR SHIFT (MAYBE USE UNSIGNED)
void solve_crossword(char*** crossword, int crossword_size, Dictionary* bigdict, Word** words, int wordnode_count, Map*** bitmaps) {
    
    /* Allocating sets for CBJ */
    for (int i = 0 ; i < wordnode_count ; ++i) {
        words[i]->conf_set = calloc(wordnode_count, sizeof(int));
        words[i]->past_fc = calloc(wordnode_count, sizeof(int));
    }

    /* Initializing map_stack (for backtrack) */
    int max_map_size = 0;
    int map_stack_size = wordnode_count;
    for (int i = 0 ; i < wordnode_count ; ++i) {
        map_stack_size += words[i]->insecc;
        if (words[i]->map->size > max_map_size) {
            max_map_size = words[i]->map->size;
        }
    }

    Map* map_stack = calloc(map_stack_size, sizeof(Map));
    for (int i = 0 ; i < map_stack_size ; ++i) {
        map_stack[i].array = malloc(max_map_size * sizeof(long long));
    }

    int map_stack_index = 0;
    
    /* Initilizing the crossword stack */
    char*** crosswords = init_crosswords(*crossword, crossword_size, wordnode_count);
    int index = 0, prune_flag = 0;
    prop_word(words, wordnode_count, index);
    map_stack[map_stack_index].sum = words[index]->map->sum;
    map_stack[map_stack_index].size = words[index]->map->size;
    memcpy(map_stack[map_stack_index].array, words[index]->map->array, words[index]->map->size * sizeof(long long));
    ++map_stack_index;
    while (index < wordnode_count) {
        prune_flag = 0;
        /* Find word in bigdict */
        char* word_found = NULL;
        if ((word_found = find_word(bigdict[words[index]->size - 1], words[index])) == NULL) {
            if (index == 0) { /* Cannot backtrack from zero */
                fprintf(stderr, "Couldn\'t solve crossword (extra sad) :(\n");
                exit(1);
            }
            int* curr_conf_set = words[index]->conf_set;
            int* curr_past_fc = words[index]->past_fc;
            /* h <- max(max-list(conf-set[i], max-list(past-fc[i]))) */
            int jump_to = index - 1;
            for (int i = index - 1 ; i >= 0 ; --i) {
                if (curr_conf_set[i] || curr_past_fc[i]) {
                    jump_to = i;
                    break;
                }
            }
            /* past-fc union, conf_set union */
            for (int i = jump_to ; i >= 0 ; --i) {
                words[jump_to]->conf_set[i] |= curr_conf_set[i] | curr_past_fc[i];
            }
            /* remove h */
            words[jump_to]->conf_set[jump_to] = 0;
            /* backtrack */
            do {
                --map_stack_index;
                words[index]->map->sum = map_stack[map_stack_index].sum;
                memcpy(words[index]->map->array, map_stack[map_stack_index].array, words[index]->map->size * sizeof(long long));
                memset(words[index]->conf_set, 0, wordnode_count * sizeof(int));
                --index;
                /* Fixing back all maps that got ruined from the word put */
                for (int i = words[index]->insecc - 1 ; i >= 0 ; --i) {
                    Word* word_b = words[index]->insecs[i].word;
                    if (word_b->in_use == 0) {
                        word_b->past_fc[index] = 0;
                        --map_stack_index;
                        word_b->map->sum = map_stack[map_stack_index].sum;
                        memcpy(word_b->map->array, map_stack[map_stack_index].array, word_b->map->size * sizeof(long long));
                    }
                }
                words[index]->in_use = 0;
            } while (index > jump_to);
            continue;
        }
        /* Copying previous state before we write anything */
        memcpy(crosswords[index + 1][0], crosswords[index][0], crossword_size * crossword_size * sizeof(char));
        /* Write word found in copy state */
        write_word(crosswords[index + 1], words[index], word_found);
        /* Label word used */
        words[index]->in_use = 1;
        /* For every intersection in word update its map with the changed letter */
        for (int i = 0 ; i < words[index]->insecc ; ++i) {
            Intersection insec = words[index]->insecs[i];
            Word* word = insec.word;
            if (word->in_use == 0) {
                map_stack[map_stack_index].sum = word->map->sum;
                map_stack[map_stack_index].size = word->map->size;
                memcpy(map_stack[map_stack_index].array, word->map->array, word->map->size * sizeof(long long));
                ++map_stack_index;
                join_map(word->map, &bitmaps[word->size - 1][insec.pos][(int)crosswords[index + 1][insec.x][insec.y]]);
                word->past_fc[index] = 1;
                /* If some map turns out to be 0 do early backtrack (pruning the domain) */
                if (sum_bit(word->map) == 0) {

                    /* conf-set[i] <- union(conf-set[i], past-fc[j]) */ 
                    for (int k = 0 ; k < index ; ++k) {
                        words[index]->conf_set[k] |= word->past_fc[k];
                    }
                    /* end of union */

                    for (int j = i ; j >= 0 ; --j) {
                        Word* word_b = words[index]->insecs[j].word;
                        word_b->past_fc[index] = 0;
                        if (word_b->in_use == 0) {
                            --map_stack_index;
                            word_b->map->sum = map_stack[map_stack_index].sum;
                            memcpy(word_b->map->array, map_stack[map_stack_index].array, word_b->map->size * sizeof(long long));
                            word_b->conf_set[index] = 0;
                        }
                    }
                    words[index]->in_use = 0;
                    --index;
                    prune_flag = 1;
                    break;
                }
            }
        }
        ++index;
        if (prune_flag == 0) {
            if (index == wordnode_count) break;
            prop_word(words, wordnode_count, index);
            map_stack[map_stack_index].sum = words[index]->map->sum;
            map_stack[map_stack_index].size = words[index]->map->size;
            memcpy(map_stack[map_stack_index].array, words[index]->map->array, words[index]->map->size * sizeof(long long));
            ++map_stack_index;
        }
    }
    *crossword = crosswords[wordnode_count];

    /* Cleanup */
    for (int i = 0 ; i < wordnode_count ; ++i) {
        free(*crosswords[i]);
        free(crosswords[i]);
    }
    free(crosswords);
    for (int i = 0 ; i < map_stack_size ; ++i) {
        free(map_stack[i].array);
    }
    free(map_stack);
    
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
    free(*crossword);
    free(crossword);
    return crosswords;
}
