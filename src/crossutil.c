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

//FIXME later
// int check_crossword(Dictionary* dictionary, char** crossword, int crossward_size, Bitmaps maps, int* map_sizes, int wordnode_count) {

//     char* buffer = malloc(sizeof(char) * 81);
//     if (buffer == NULL) /* Malloc error handling */
//         error("Error while allocating memory", errno);

//     int count = 0;
//     while (fscanf(stdin, "%80s", buffer) == 1) {
//         int word_size = strlen(buffer);
//         if (words[0][count].end - words[0][count].begin + 1 != word_size) {
//             fprintf(stderr, "Word \"%s\" could not be placed\n", buffer);
//             free(buffer);
//             return 1;
//         }
//         int* map = create_map(maps, map_sizes, buffer);
//         if (find_word(dictionary[word_size - 1], map, map_sizes[word_size - 1]) == NULL) {
//             fprintf(stderr, "Word \"%s\" not in dictionary", buffer);
//             free(buffer);
//             return 1;
//         }
//         write_word(crossword, words[0][count], buffer);
//         count++;
//     }

//     if (count != hor_count) {
//         fprintf(stderr, "Not enough words");
//         return 1;
//     }

//     for(int i = 0; i < ver_count; i++){
//         Word word = words[1][i];
//         int k = 0;
//         for (int j = word.begin ; j < word.end ; j++, k++){
//             buffer[k] = crossword[j][word.constant];
//         }
//         buffer[k] = '\0';
//         int* map = create_map(maps, map_sizes, buffer);
//         int word_size = strlen(buffer);
//         if (find_word(dictionary[word_size - 1], map, map_sizes[word_size - 1]) == NULL) {
//             fprintf(stderr, "Word \"%s\" not in dictionary", buffer);
//             free(buffer);
//             return 1;
//         }
//     }
//     free(buffer);
//     return 0;
// }


//TODO break struct State into two different stacks
void solve_crossword(char*** crossword, int crossword_size, Dictionary* bigdict, Wordnode words, 
                     int wordnode_count, Bitmaps bitmaps, int* map_sizes) {
    int full_map_size = 0;
    for (int j = 0 ; j < wordnode_count ; ++j) {
        full_map_size += map_sizes[words[j].size - 1];
    }
    char*** crosswords = NULL;
    int** maps = NULL;
    init_states(&crosswords, &maps, *crossword, crossword_size, words, wordnode_count, bitmaps, map_sizes, full_map_size);
    int index = 0;
    prop_word(maps, words, index, map_sizes, wordnode_count);
    while (index < wordnode_count) {
        /* Find word in bigdict */
        char* word_found;
        if ((word_found = find_word(bigdict[words[index].size - 1], maps[index], map_sizes[words[index].size - 1])) == NULL) {
            if (index == 0) {
                fprintf(stderr, "what happend\n");
                exit(1);
            }
            --index;
            continue;
        }
        if (index == wordnode_count - 1) break;
        memcpy(crosswords[index + 1][0], crosswords[index][0], crossword_size * crossword_size * sizeof(char));
        write_word(crosswords[index + 1], words[index], word_found);
        ++index;
        for (int i = index ; i < wordnode_count ; ++i) { //TODO make maps structs that contain bit sum
            update_map(crosswords[index], maps[i], map_sizes[words[i].size - 1], words[i], bitmaps);
        }
        prop_word(maps, words, index, map_sizes, wordnode_count);
    }
    *crossword = crosswords[wordnode_count - 1];
}

void init_states(char**** crosswords_ret, int*** maps_ret, char** crossword, int crossword_size, Wordnode words, 
                     int wordnode_count, Bitmaps bitmaps, int* map_sizes, int full_map_size) 
{
    /* Crosswords initialization */
    char*** crosswords = malloc(wordnode_count * sizeof(char**));
    if (crosswords == NULL) /* Malloc error handling */
        error("Error while allocating memory", errno);
    for (int i = 0 ; i < wordnode_count ; ++i) {
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

    /* Maps initialization */
    int** maps = malloc(wordnode_count * sizeof(int*));
    if (maps == NULL) /* Malloc error handling */
        error("Error while allocating memory", errno);
    maps[0] = malloc(full_map_size * sizeof(int));
    if (maps[0] == NULL) /* Malloc error handling */
        error("Error while allocating memory", errno);
    
    int map_index = map_sizes[words[0].size - 1];
    for (int j = 1 ; j < wordnode_count ; ++j) {
        maps[j] = maps[0] + map_index;
        map_index += map_sizes[words[j].size - 1];
    }

    /* Initial copy */
    memcpy(crosswords[0][0], crossword[0], crossword_size * crossword_size * sizeof(char));
    for (int j = 0 ; j < wordnode_count ; ++j) {
        int size = words[j].size;
        memcpy(maps[j], bitmaps[size - 1][size][0], map_sizes[size - 1] * sizeof(int));
    }

    *crosswords_ret = crosswords;
    *maps_ret = maps;
    return;
}
