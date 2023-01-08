#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "extratypes.h"
#include "extrafuns.h"

extern int errno;

Dictnode* make_dict(char* dictionary_path, int max_word_size, int** words_count) {

    FILE* dictionary_file = fopen(dictionary_path, "r");
    int* words_count_l = calloc(max_word_size, sizeof(int));
    char* buffer = malloc(81 * sizeof(char));
    while (fscanf(dictionary_file, "%80s", buffer) == 1) {
        int word_size = strlen(buffer);
        if (word_size > max_word_size) continue;
        words_count_l[word_size - 1]++;
    }
    *words_count = words_count_l;

    rewind(dictionary_file); /* Go back to the top of dictionary stream */

    /* Allocate enough linked lists for all needed word sizes */
    Dictnode* dictionary = malloc(max_word_size * sizeof(Dictnode));

    for (int i = 0 ; i < max_word_size ; i++) {
        dictionary[i] = malloc(words_count_l[i] * sizeof(Dictionary));
    }

    int* words_count_new = calloc(max_word_size, sizeof(int));
    while (fscanf(dictionary_file, "%80s", buffer) == 1) { /* Scan 1 word at a time */
        int word_size = strlen(buffer);
        if (word_size > max_word_size) continue; /* No need to allocate larger words than needed */
        int word_value = word_val(buffer);
        int index = words_count_new[word_size - 1];
        dictionary[word_size - 1][index].word = malloc((word_size + 1) * sizeof(char)); /* Allocate memory for word */

        strcpy(dictionary[word_size - 1][index].word, buffer); /* Copy word in buffer to node */
        dictionary[word_size - 1][index].value = word_value;

        words_count_new[word_size - 1]++;
    }
    for (int i = 0 ; i < max_word_size ; ++i) {
        sort_dict(dictionary[i], 0, words_count_new[i] - 1);
    }
    fclose(dictionary_file);
    free(buffer);
    return dictionary;
}

void print_dict(Dictnode* dictionary, int max_word_size, int* words_count) {
    for (int i = 0 ; i < max_word_size ; ++i) {
        printf("words with size: %d\n\n", i + 1);
        for (int j = 0 ; j < words_count[i] ; ++j) {
            printf("%s\n", dictionary[i][j].word);
        }
    }
}

void free_dict(Dictnode* dictionary, int max_word_size) {
    for (int i = 0 ; i < max_word_size ; i++) {
        free(dictionary[i]);
    }
    free(dictionary);
}

char* find_word(Dictnode subdict, int* map, int map_size) {
    if (!map) return NULL;
    char* word;
    for (int i = 0 ; i < map_size ; ++i) {
        if (map[i] == 0) continue;
        for (int j = 0 ; j < 32 ; j++) {
            if ((map[i] >> j) & 1) {
                map[i] ^= 1 << j;
                return subdict[i * 32 + j].word;
            }
        }
    }
    return NULL;
}

int word_val(char* word) {
    int len = strlen(word);
    int value = 0;
    for (int i = 0 ; i < len ; i++) {
        switch (word[i]) {
            case 'e': value += 26; break;
            case 'a': value += 25; break;
            case 'i': value += 24; break;
            case 'r': value += 23; break;
            case 't': value += 22; break;
            case 'o': value += 21; break;
            case 'n': value += 20; break;
            case 's': value += 19; break;
            case 'l': value += 18; break;
            case 'c': value += 17; break;
            case 'u': value += 16; break;
            case 'm': value += 15; break;
            case 'd': value += 14; break;
            case 'p': value += 13; break;
            case 'h': value += 12; break;
            case 'g': value += 11; break;
            case 'b': value += 10; break;
            case 'y': value += 9; break;
            case 'f': value += 8; break;
            case 'w': value += 7; break;
            case 'k': value += 6; break;
            case 'v': value += 5; break;
            case 'x': value += 4; break;
            case 'z': value += 3; break;
            case 'j': value += 2; break;
            case 'q': value += 1; break;
        }
    }
    return value;
}

void sort_dict(Dictnode subdict, int first, int last) {
    int i, j, pivot;
    Dictionary temp;
    if (first < last) {
        pivot = first;
        i = first;
        j = last;
        while (i < j) {
            while (subdict[i].value >= subdict[pivot].value && i < last) i++;
            while (subdict[j].value < subdict[pivot].value) j--;
            if (i < j) {
                temp = subdict[i];
                subdict[i] = subdict[j];
                subdict[j] = temp;
            }
        }
        temp = subdict[pivot];
        subdict[pivot] = subdict[j];
        subdict[j] = temp;
        sort_dict(subdict, first, j - 1);
        sort_dict(subdict, j + 1 , last);
    }
}