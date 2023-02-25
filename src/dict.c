#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "extratypes.h"
#include "extrafuns.h"

extern int errno;

Dictionary* init_dictionary(char* dictionary_path, int max_word_size, char** all_of_dict_ret,
                            int** dict_count_ret, int* lengths_on_grid, int* ascii_on_dict) {

    FILE* dictionary_file = fopen(dictionary_path, "r");
    if (dictionary_file == NULL) /* File error handling */
        error("Error while handling dictionary", errno);

    /* New method */
    fseek(dictionary_file, 0, SEEK_END);
    int file_size = ftell(dictionary_file);
    fseek(dictionary_file, 0, SEEK_SET);
    char* all_of_dict = malloc((file_size + 1) * sizeof(char));
    fread(all_of_dict, sizeof(char), file_size, dictionary_file);
    all_of_dict[file_size] = 0;
    
    /* Finding how many words (per length) dict has */
    int* dict_count = calloc(max_word_size, sizeof(int));
    mallerr(dict_count, errno);

    /* Intiializing worth */
    int worth[256] = {0};

    /* Counting the words in dict file */
    int word_size = 0;
    for (int i = 0 ; i < file_size ; ++i) {
        if (all_of_dict[i] == '\n') {
            if (word_size <= max_word_size && lengths_on_grid[word_size - 1]) {
                ++dict_count[word_size - 1];
            }
            else {
                for (int j = i - word_size ; j < i ; ++j) {
                    --worth[(int)all_of_dict[j]];
                }
            }
            word_size = 0;
            continue;
        }
        ascii_on_dict[(int)all_of_dict[i]] = 1;
        ++worth[(int)all_of_dict[i]];
        ++word_size;
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
    
    /* Scanning words to put into dictionary */
    char* token = strtok(all_of_dict, "\n");
    while (token) { /* Scan 1 word at a time */
        int word_size = strlen(token);
        // fprintf(stderr, "%s\n", token);
        if (word_size > max_word_size || lengths_on_grid[word_size - 1] == 0) {
            token = strtok(NULL, "\n");
            continue;
        }

        int index = index_array[word_size - 1];
        bigdict[word_size - 1][index] = token; /* Copy word in buffer to dict */
        dictnode_values[word_size - 1][index] = word_val(token, worth); /* Saving the words value */

        ++index_array[word_size - 1];
        token = strtok(NULL, "\n");
    }
    for (int i = 0 ; i < max_word_size ; ++i) {
        if (lengths_on_grid[i] == 0) continue;
        sort_dictionary(bigdict[i], dictnode_values[i], 0, index_array[i] - 1);
    }

    /* Returning the values */
    *dict_count_ret = dict_count;
    *all_of_dict_ret = all_of_dict;
    /* Cleanup */
    for (int i = 0 ; i < max_word_size ; ++i) {
        free(dictnode_values[i]);
    }
    free(dictnode_values);
    free(index_array);
    fclose(dictionary_file);
    return bigdict;
}

void free_dictionary(Dictionary* bigdict, int max_word_size, char* all_of_dict) {
    free(all_of_dict);
    for (int i = 0 ; i < max_word_size ; ++i) {
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

