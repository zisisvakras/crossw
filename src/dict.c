#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "extratypes.h"
#include "extrafuns.h"

Dictionary* init_dictionary(char* dictionary_path, int max_word_size, char** all_of_dict_ret,
                            int** dict_count_ret, register int* lengths_on_grid, register int* ascii_on_dict) {

    FILE* dictionary_file = fopen(dictionary_path, "r");
    if (dictionary_file == NULL) /* File error handling */
        error("Error while handling dictionary");

    /* Creating a big memory block that has the whole dictionary file */
    fseek(dictionary_file, 0, SEEK_END);
    int file_size = ftell(dictionary_file);
    fseek(dictionary_file, 0, SEEK_SET);
    register char* all_of_dict = malloc((file_size + 1) * sizeof(char));
    mallerr(all_of_dict);
    if (fread(all_of_dict, sizeof(char), file_size, dictionary_file) != (size_t)file_size)
        error("Error while reading the dictionary");
    all_of_dict[file_size] = '\0';
    
    /* Finding how many words (per length) dict has */
    register int* dict_count = calloc(max_word_size, sizeof(int));
    mallerr(dict_count);

    /* Intiializing worth */
    int worth[256] = {0};

    /* Counting the words in dict file */
    int word_size = 0;
    for (register int i = 0 ; i < file_size ; ++i) {
        if (all_of_dict[i] == '\n') {
            if (word_size <= max_word_size && lengths_on_grid[word_size - 1]) {
                ++dict_count[word_size - 1];
            }
            else {
                /* If word was not needed remove it from worth */
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
    mallerr(bigdict);
    int** dictnode_values = calloc(max_word_size, sizeof(int*));
    mallerr(dictnode_values);
    for (int i = 0 ; i < max_word_size ; ++i) {
        if (lengths_on_grid[i] == 0) continue;
        bigdict[i] = malloc(dict_count[i] * sizeof(char*));
        mallerr(bigdict[i]);
        dictnode_values[i] = malloc(dict_count[i] * sizeof(int*));
        mallerr(dictnode_values[i]);
    }

    /* Keeping track of all array indexes */
    int* index_array = calloc(max_word_size, sizeof(int));
    mallerr(index_array);
    
    /* Scanning words to put into dictionary */
    char* token = strtok(all_of_dict, "\n");
    while (token) { /* While strtok finds tokens */
        int word_size = strlen(token);
        if (word_size > max_word_size || lengths_on_grid[word_size - 1] == 0) {
            token = strtok(NULL, "\n");
            continue;
        }

        int index = index_array[word_size - 1];
        bigdict[word_size - 1][index] = token; /* Saving the token */
        dictnode_values[word_size - 1][index] = word_val(token, worth);

        ++index_array[word_size - 1];
        token = strtok(NULL, "\n");
    }

    /* Sorting the necessary dicts */
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

char* find_word(Dictionary dictionary, Word* word) {
    register unsigned long long* array = word->map->array;
    int size = word->map->size;
    for (register int i = 0 ; i < size ; ++i) {
        if (array[i] == 0) continue;
        for (register int j = 0 ; j < 64 ; ++j) {
            if ((array[i] >> j) & 1) {
                /* Turning off the bit so we don't use it in the future */
                array[i] ^= 1ULL << j;
                --word->map->sum;
                return dictionary[(i << 6) | j];
            }
        }
    }
    return NULL;
}

int word_val(register char* word, register int* worth) {
    register int value = 0, i = -1;
    while (word[++i]) {
        value += worth[(int)word[i]];
    }
    return value;
}

/* Quick sort implementation that sorts the dict words based on the word's worth */
void sort_dictionary(Dictionary dictionary, register int* dictnode_values, int first, int last) {
    register int i, j, pivot;
    register char* temp;
    register int temp_v;
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

