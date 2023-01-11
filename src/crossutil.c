#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "extratypes.h"
#include "extrafuns.h"

extern int errno;

int init_crossword(char* crossword_path, char*** crossword, int* crossword_size, int* max_word_size) {
    FILE* crossword_file = fopen(crossword_path, "r");
    if (crossword_file == NULL) { /* File error handling */
        fprintf(stderr, "Error while handling crossword: %s", strerror(errno));
        return errno;
    }
    //TODO errno and return
    if (fscanf(crossword_file, "%d", crossword_size) != 1) {
        fprintf(stderr, "Error while reading size of crossword"); /* If scan didn't read 1 integer throw error */
        return errno;
    }
    
    //TODO check null pointer
    (*crossword) = malloc((*crossword_size) * sizeof(char*));
    if (*crossword == NULL) { /* Malloc error handling */
        fprintf(stderr, "Error while allocating memory: %s", strerror(errno));
        return 1;
    }

    for (int i = 0 ; i < (*crossword_size) ; i++) {
        (*crossword)[i] = malloc((*crossword_size) * sizeof(char));
        if ((*crossword)[i] == NULL) { /* Malloc error handling */
            fprintf(stderr, "Error while allocating memory: %s", strerror(errno));
            return errno;
        }

        for (int j = 0 ; j < (*crossword_size) ; j++) {
            (*crossword)[i][j] = '-';
        }
    }

    int x, y;
    while (fscanf(crossword_file, "%d %d", &x, &y) == 2) { /* While it continues to read black tiles */
        (*crossword)[x - 1][y - 1] = '#';
    }
    /* Biggest word finder */
    for (int i = 0 ; i < (*crossword_size) ; i++) {
        int len_row = 0;
        int len_col = 0;
        for (int j = 0 ; j < (*crossword_size) ; j++) {
            if ((*crossword)[i][j] == '#') {
                if (len_row > (*max_word_size)) (*max_word_size) = len_row;
                len_row = 0;
            }
            if ((*crossword)[i][j] == '-') len_row++;
            if ((*crossword)[j][i] == '#') {
                if (len_col > (*max_word_size)) (*max_word_size) = len_col;
                len_col = 0;
            }
            if ((*crossword)[j][i] == '-') len_col++;
        }
        if (len_row > (*max_word_size)) (*max_word_size) = len_row;
        if (len_col > (*max_word_size)) (*max_word_size) = len_col;
    }
    fclose(crossword_file);
    return 0;
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

//FIXME LATER
// int check_crossword(char** crossword, int crossward_size, Wordnode* words, int wordnode_count, Dictnode* dictionary, Bitmaps maps, int* map_sizes) {
//     char* buffer = malloc(sizeof(char) * 81);
//     if (buffer == NULL) { /* Malloc error handling */
//         fprintf(stderr, "Error while allocating memory: %s", strerror(errno));
//         return errno;
//     }

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

//TODO after finished project check if filters are getting freed
char* create_filter(char** crossword, Word word) {
    int flag = word.orientation;
    int filter_size = word.end - word.begin + 1;
    char* filter = malloc((filter_size + 1) * sizeof(char));
    if (filter == NULL) { /* Malloc error handling */
        fprintf(stderr, "Error while allocating memory: %s", strerror(errno));
        exit(1);
    }

    if (flag) {
        for (int i = word.begin, j = 0 ; i <= word.end ; i++, j++) {
            char temp = crossword[i][word.constant];
            filter[j] = temp == '-' ? '?' : temp;
        }
    }
    else {
        for (int i = word.begin, j = 0 ; i <= word.end ; i++, j++) {
            char temp = crossword[word.constant][i];
            filter[j] = temp == '-' ? '?' : temp;
        }
    }
    filter[filter_size] = '\0';
    return filter;
}


void solve_crossword(char** crossword, Dictnode* dictionary, Wordnode words, int wordnode_count, Bitmaps maps, int* map_sizes) {
    Actionnode actions = NULL;
    int* map = NULL;
    prop_word(words, wordnode_count - 1, crossword, maps, map_sizes);
    while (wordnode_count) {
        /* Find word in dictionary */
        char* filter = create_filter(crossword, words[wordnode_count - 1]); //TODO optimize filter
        int word_size = strlen(filter);
        if (!map) map = create_map(maps, map_sizes, filter);
        char* word_found;
        if ((word_found = find_word(dictionary[word_size - 1], map, map_sizes[word_size - 1])) == NULL) {
            int* old_map = NULL;
            char* changed = NULL;
            Wordnode wordnode = NULL;
            if (!actions) {
                fprintf(stderr, "what happend\n");
                exit(1);
            }
            pop_word(&actions, &old_map, &changed, &wordnode);
            wordnode_count++;
            delete_word(crossword, *wordnode, changed);
            free(map);
            map = old_map;
            free(changed);
            free(filter);
            continue;
        }
        char* changed = word_written(word_found, filter);
        push_word(&actions, map, changed, &words[wordnode_count - 1]);
        write_word(crossword, words[wordnode_count - 1], word_found);
        map = NULL;
        wordnode_count--;
        if (wordnode_count) prop_word(words, wordnode_count - 1, crossword, maps, map_sizes);
        free(filter);
    }
}
