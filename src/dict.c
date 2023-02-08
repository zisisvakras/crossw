#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "extratypes.h"
#include "extrafuns.h"

extern int errno;

Dictionary* init_dictionary(char* dictionary_path, int max_word_size, int** words_count_ret) {

    FILE* dictionary_file = fopen(dictionary_path, "r");
    if (dictionary_file == NULL) /* File error handling */
        error("Error while handling dictionary", errno);

    /* Allocating a buffer */
    char* buffer = malloc(81 * sizeof(char));
    if (buffer == NULL) /* Malloc error handling */
        error("Error while allocating memory", errno);
    
    /* Finding how many words (per length) dict has */
    int* words_count = calloc(max_word_size, sizeof(int));
    if (words_count == NULL) /* Calloc error handling */
        error("Error while allocating memory", errno);

    while (fscanf(dictionary_file, "%80s", buffer) == 1) {
        int word_size = strlen(buffer);
        if (word_size > max_word_size) continue;
        words_count[word_size - 1]++;
    }

    /* Returning words_count for future use */
    *words_count_ret = words_count;

    /* Go back to the top of dictionary stream */
    rewind(dictionary_file);

    /* Allocate enough arrays for all needed word sizes */
    Dictionary* bigdict = malloc(max_word_size * sizeof(Dictionary));
    int** dictnode_values = malloc(max_word_size * sizeof(int*));
    if (bigdict == NULL || dictnode_values == NULL) /* Malloc error handling */
        error("Error while allocating memory", errno);

    for (int i = 0 ; i < max_word_size ; ++i) {
        bigdict[i] = malloc(words_count[i] * sizeof(char*));
        dictnode_values[i] = malloc(words_count[i] * sizeof(int*));
        if (bigdict[i] == NULL || dictnode_values[i] == NULL) /* Malloc error handling */
            error("Error while allocating memory", errno);
    }

    /* Keeping track of all array indexes */
    int* index_array = calloc(max_word_size, sizeof(int));
    if (index_array == NULL) /* Calloc error handling */
        error("Error while allocating memory", errno);

    while (fscanf(dictionary_file, "%80s", buffer) == 1) { /* Scan 1 word at a time */
        int word_size = strlen(buffer);
        if (word_size > max_word_size) continue; /* No need to allocate larger words than needed */
        int index = index_array[word_size - 1];
        bigdict[word_size - 1][index] = malloc((word_size + 1) * sizeof(char)); /* Allocate memory for word */
        if (bigdict[word_size - 1][index] == NULL) /* Malloc error handling */
            error("Error while allocating memory", errno);

        strcpy(bigdict[word_size - 1][index], buffer); /* Copy word in buffer to node */
        dictnode_values[word_size - 1][index] = word_val(buffer);

        index_array[word_size - 1]++;
    }
    for (int i = 0 ; i < max_word_size ; ++i) {
        sort_dictionary(bigdict[i], dictnode_values[i], 0, index_array[i] - 1);
    }
    for (int i = 0 ; i < max_word_size ; ++i) {
        free(dictnode_values[i]);
    }
    free(dictnode_values);
    fclose(dictionary_file);
    free(index_array);
    free(buffer);
    return bigdict;
}

/**
 * @details debug tool
*/
void print_dictionary(Dictionary* bigdict, int max_word_size, int* words_count) {
    for (int i = 0 ; i < max_word_size ; ++i) {
        printf("words with size: %d\n\n", i + 1);
        for (int j = 0 ; j < words_count[i] ; ++j) {
            printf("%s\n", bigdict[i][j]);
        }
    }
}

void free_dictionary(Dictionary* bigdict, int max_word_size, int* words_count) {
    for (int i = 0 ; i < max_word_size ; i++) {
        for (int j = 0 ; j < words_count[i] ; ++j) {
            free(bigdict[i][j]);
        }
        free(bigdict[i]);
    }
    free(bigdict);
}

//TODO fix 32 and make it sizeof(int) * 8
char* find_word(Dictionary dictionary, Word* word) {
    int* array = word->map->array;
    int size = word->map->size;
    for (int i = 0 ; i < size ; ++i) {
        if (array[i] == 0) continue;
        for (int j = 0 ; j < 32 ; ++j) {
            if ((array[i] >> j) & 1) {
                array[i] ^= 1 << j;
                return dictionary[(i << 5) | j];
            }
        }
    }
    return NULL;
}

int word_val(char* word) {
    int value = 0, i = -1;
    int worth['z' + 1] = {
        ['e'] = 26, ['a'] = 25, ['i'] = 24, ['r'] = 23,
        ['t'] = 22, ['o'] = 21, ['n'] = 20, ['s'] = 19,
        ['l'] = 18, ['c'] = 17, ['u'] = 16, ['m'] = 15,
        ['d'] = 14, ['p'] = 13, ['h'] = 12, ['g'] = 11,
        ['b'] = 10, ['y'] = 9,  ['f'] = 8,  ['w'] = 7,
        ['k'] = 6,  ['v'] = 5,  ['x'] = 4,  ['z'] = 3,
        ['j'] = 2,  ['q'] = 1
    };
    while (word[++i]) {
        value += worth[(int)word[i]];
    }
    return value;
}

void sort_dictionary(Dictionary dictionary, int* dictnode_values, int first, int last) {
    int i, j, pivot;
    char* temp;
    int temp_v;
    if (first < last) {
        pivot = first;
        i = first;
        j = last;
        while (i < j) {
            while (dictnode_values[i] >= dictnode_values[pivot] && i < last) i++;
            while (dictnode_values[j] < dictnode_values[pivot]) j--;
            if (i < j) {
                temp = dictionary[i];
                dictionary[i] = dictionary[j];
                dictionary[j] = temp;
                temp_v = dictnode_values[i];
                dictnode_values[i] = dictnode_values[j];
                dictnode_values[j] = temp_v;
            }
        }
        temp = dictionary[pivot];
        dictionary[pivot] = dictionary[j];
        dictionary[j] = temp;
        temp_v = dictnode_values[pivot];
        dictnode_values[pivot] = dictnode_values[j];
        dictnode_values[j] = temp_v;
        sort_dictionary(dictionary, dictnode_values, first, j - 1);
        sort_dictionary(dictionary, dictnode_values, j + 1 , last);
    }
}