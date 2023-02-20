#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "extratypes.h"
#include "extrafuns.h"

extern int errno;

Dictionary* init_dictionary(char* dictionary_path, int max_word_size, int** dict_count_ret, int** multi) {

    srand(20001);

    FILE* dictionary_file = fopen(dictionary_path, "r");
    if (dictionary_file == NULL) /* File error handling */
        error("Error while handling dictionary", errno);

    /* Allocating a buffer */
    char* buffer = malloc(81 * sizeof(char));
    mallerr(buffer, errno);
    
    /* Finding how many words (per length) dict has */
    int* dict_count = calloc(max_word_size, sizeof(int));
    mallerr(dict_count, errno);

    /* Counting the words in dict file */
    while (fscanf(dictionary_file, "%80s", buffer) == 1) {
        int word_size = strlen(buffer);
        if (word_size > max_word_size) continue;
        dict_count[word_size - 1]++;
    }

    //TODO remove useless allocated dicts
    /* Allocate enough arrays for all word sizes that we may need */
    Dictionary* bigdict = malloc(max_word_size * sizeof(Dictionary));
    mallerr(bigdict, errno);
    int** dictnode_values = malloc(max_word_size * sizeof(int*));
    mallerr(dictnode_values, errno);
    for (int i = 0 ; i < max_word_size ; ++i) {
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

        int index = index_array[word_size - 1];
        bigdict[word_size - 1][index] = malloc((word_size + 1) * sizeof(char)); /* Allocate memory for word */
        mallerr(bigdict[word_size - 1][index], errno);

        strcpy(bigdict[word_size - 1][index], buffer); /* Copy word in buffer to dict */
        dictnode_values[word_size - 1][index] =  /*rand();*/word_val(buffer, multi); /* Saving the words value */

        ++index_array[word_size - 1];
    }
    for (int i = 0 ; i < max_word_size ; ++i) {
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
    for (int i = 0 ; i < max_word_size ; i++) {
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
    int* array = word->map->array;
    int size = word->map->size;
    int offset = word->map->offset >> 5;
    for (int i = offset ; i < size ; ++i) {
        if (array[i] == 0) continue;
        int n = array[i];
        for (int j = 0 ; j < 32 ; ++j) {
            if ((n >> j) & 1) {
                array[i] ^= 1 << j;
                --word->map->sum;
                word->map->offset = ((i << 5) | j) + 1;
                return dictionary[(i << 5) | j];
            }
        }
    }
    return NULL;
}

//TODO ask takis if letters other than 'a'-'z' are allowed
//TODO making maps for all ascii characters
int word_val(char* word, int** multi) {
    int* smulti = multi[strlen(word) - 1]; // "s"pecific multi
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
        value += worth[(int)word[i]] * smulti[i];
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