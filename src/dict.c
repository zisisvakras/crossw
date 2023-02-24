#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "extratypes.h"
#include "extrafuns.h"

extern int errno;

Dictionary* init_dictionary(char* dictionary_path, int max_word_size,
                            int** dict_count_ret, int* lengths_on_grid, int* ascii_on_dict) {

    FILE* dictionary_file = fopen(dictionary_path, "r");
    if (dictionary_file == NULL) /* File error handling */
        error("Error while handling dictionary", errno);

    /* Allocating a buffer */
    char* buffer = malloc(81 * sizeof(char));
    mallerr(buffer, errno);
    
    /* Finding how many words (per length) dict has */
    int* dict_count = calloc(max_word_size, sizeof(int));
    mallerr(dict_count, errno);

    /* Intiializing worth */
    int worth[256] = {0};

    /* Counting the words in dict file */
    while (fscanf(dictionary_file, "%80s", buffer) == 1) {
        int word_size = strlen(buffer);
        if (word_size > max_word_size) continue;
        if (lengths_on_grid[word_size - 1] == 0) continue;
        for (int i = 0 ; i < word_size ; ++i) {
            ascii_on_dict[(int)buffer[i]] = 1;
            ++worth[(int)buffer[i]];
        }
        dict_count[word_size - 1]++;
    }

    /* Allocate enough arrays for all word sizes that we may need */
    Dictionary* bigdict = calloc(max_word_size, sizeof(Dictionary));
    mallerr(bigdict, errno);
    int** dictnode_values = calloc(max_word_size, sizeof(int*));
    mallerr(dictnode_values, errno);
    for (int i = 0 ; i < max_word_size ; ++i) {
        if (lengths_on_grid[i] == 0) continue;
        bigdict[i] = malloc(dict_count[i] * sizeof(char*));
        mallerr(bigdict[i], errno);
        dictnode_values[i] = malloc(dict_count[i] * sizeof(int*));
        mallerr(dictnode_values[i], errno);
    }

    /* Keeping track of all array indexes */
    int* index_array = calloc(max_word_size, sizeof(int));
    mallerr(index_array, errno);

    /* Go back to the top of dictionary stream */
    rewind(dictionary_file);

    /* Scanning words to put into dictionary */
    while (fscanf(dictionary_file, "%80s", buffer) == 1) { /* Scan 1 word at a time */
        int word_size = strlen(buffer);
        if (word_size > max_word_size) continue; /* No need to allocate larger words than needed */
        if (lengths_on_grid[word_size - 1] == 0) continue;

        int index = index_array[word_size - 1];
        bigdict[word_size - 1][index] = malloc((word_size + 1) * sizeof(char)); /* Allocate memory for word */
        mallerr(bigdict[word_size - 1][index], errno);

        strcpy(bigdict[word_size - 1][index], buffer); /* Copy word in buffer to dict */
        dictnode_values[word_size - 1][index] = word_val(buffer, worth); /* Saving the words value */

        ++index_array[word_size - 1];
    }
    for (int i = 0 ; i < max_word_size ; ++i) {
        if (lengths_on_grid[i] == 0) continue;
        sort_dictionary(bigdict[i], dictnode_values[i], 0, index_array[i] - 1);
    }

    /* Returning the values */
    *dict_count_ret = dict_count;
    /* Cleanup */
    for (int i = 0 ; i < max_word_size ; ++i) {
        free(dictnode_values[i]);
    }
    free(dictnode_values);
    free(index_array);
    free(buffer);
    fclose(dictionary_file);
    return bigdict;
}

void free_dictionary(Dictionary* bigdict, int max_word_size, int* dict_count) {
    for (int i = 0 ; i < max_word_size ; ++i) {
        for (int j = 0 ; j < dict_count[i] ; ++j) {
            free(bigdict[i][j]);
        }
        free(bigdict[i]);
    }
    free(bigdict);
}


//TODO add offset to maps when u find a word so you can start from there
//TODO maybe add them to premade bitmaps as well (that will be calculated in sum_bit)
char* find_word(Dictionary dictionary, Word* word) {
    long long* array = word->map->array;
    int size = word->map->size;
    for (int i = 0 ; i < size ; ++i) {
        if (array[i] == 0) continue;
        for (int j = 0 ; j < 64 ; ++j) {
            if ((array[i] >> j) & 1) {
                array[i] ^= 1LL << j;
                --word->map->sum;
                return dictionary[(i << 6) | j];
            }
        }
    }
    return NULL;
}

int word_val(char* word, int* worth) {
    int value = 0, i = -1;
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
            while (dictnode_values[i] >= dictnode_values[pivot] && i < last) ++i;
            while (dictnode_values[j] < dictnode_values[pivot]) --j;
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