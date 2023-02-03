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

    for (int i = 0 ; i < (*crossword_size) ; ++i) {
        (*crossword)[i] = malloc((*crossword_size) * sizeof(char));
        if ((*crossword)[i] == NULL) /* Malloc error handling */
            error("Error while allocating memory", errno);

        memset((*crossword)[i], '-', (*crossword_size) * sizeof(char));
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


//TODO try writeall instead of cpy
//TODO malloc checks, wordnode_count -> index
void solve_crossword(char*** crossword, Dictionary* bigdict, Wordnode words, int wordnode_count, Bitmaps maps, int* map_sizes) {
    Action* actions = init_actions();
    int max = wordnode_count;
    int* map = NULL;
    prop_word(words, wordnode_count - 1, crossword, maps, map_sizes);
    while (wordnode_count) {
        /* Find word in bigdict */
        Word word =  words[wordnode_count - 1]; //TODO optimize filter
        int word_size = word.end - word.begin + 1;
        map = actions[wordnode_count].map[wordnode_count - 1];
        char* word_found;
        if ((word_found = find_word(bigdict[word_size - 1], map, map_sizes[word_size - 1])) == NULL) {
            if (wordnode_count == max) {
                fprintf(stderr, "what happend\n");
                exit(1);
            }
            wordnode_count++;
            int* old_map = actions[wordnode_count].map;
            char* changed = actions[wordnode_count].changed;
            Wordnode wordnode = actions[wordnode_count].wordnode;
            delete_word(crossword, *wordnode, changed);
            free(map);
            map = old_map;
            free(changed);
            free(filter);
            continue;
        }
        wordnode_count--;
        actioncpy(actions[wordnode_count], actions[wordnode_count + 1]);
        write_word(actions[wordnode_count].crossword, words[wordnode_count - 1], word_found);
        //MAP UPDATES
        if (wordnode_count) prop_word(words, wordnode_count - 1, crossword, maps, map_sizes);
    }
    *crossword = actions[1].crossword;
}

//TODO make actions wordnode_count not +1
//TODO malloc checks
Action* init_actions(char** crossword, int crossword_size, Dictionary* bigdict, Wordnode words, 
                     int wordnode_count, Bitmaps maps, int* map_sizes) 
{
    Action* actions = malloc((wordnode_count + 1) * sizeof(Action));
    for (int i = 0 ; i <= wordnode_count ; ++i) {
        actions[i].map = malloc((wordnode_count) * sizeof(int*));
        for (int j = 0 ; j < wordnode_count ; ++j) {
            int word_size = words[j].end - words[j].begin + 1;
            int map_size = map_sizes[word_size - 1];
            actions[i].map[j] = malloc(map_size * sizeof(int));
            memcpy(actions[i].map[j], maps[word_size - 1][word_size][0], map_size * sizeof(int));
        }
        actions[i].crossword = malloc(crossword_size * sizeof(char*));
        for (int j = 0 ; j < crossword_size ; ++j) {
            actions[i].crossword[j] = malloc(crossword_size * sizeof(char));
        }
    }
    actions[wordnode_count].crossword = crossword;
    return actions;
}

void actioncpy(Action dest, Action src, int crossword_size, int wordnode_count, int* map_sizes) {

}
