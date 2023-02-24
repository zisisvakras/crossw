#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "extratypes.h"
#include "extrafuns.h"


//TODO check for negative user input
/**
 * @details debug tool
*/
void print_words(Word** words, int wordnode_count, char** crossword) {
    for (int i = 0 ; i < wordnode_count ; ++i) {
        putchar('\n');
        int c[5] = {
            words[i]->orientation,
            words[i]->constant,
            words[i]->begin,
            words[i]->end,
            words[i]->size
        };
        printf("(%p) %s: %d begin: %d end: %d size: %d\n", (void*)(words + i), c[0] ? "col" : "row", c[1], c[2], c[3], c[4]);
        print_map(words[i]->map);
        /* Interesections */
        printf("insecs:\n");
        // @deprecated 
        // while(words[i]->insecs[j].word) {
        //     printf("%p\n", (void*)(words[i]->insecs[j].word));
        //     ++j;
        // }
        /* Actual value in crossword */
        printf("value: ");
        if (words[i]->orientation) {
            for (int j = words[i]->begin ; j <= words[i]->end ; ++j) {
                putchar(crossword[j][words[i]->constant]);
            }
        }
        else {
            for (int j = words[i]->begin ; j <= words[i]->end ; ++j) {
                putchar(crossword[words[i]->constant][j]);
            }
        }
        putchar('\n');
        putchar('\n');
    }
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

/**
 * @details debug tool
*/
void print_map(Map* map) {
    long long* array = map->array;
    int size = map->size;
    int sum = map->sum;
    printf("size: %d, sum: %d\n", size, sum);
    for (int i = 0 ; i < size ; i+=4) {
        for (int j = 0 ; j < 4 ; ++j) {
            if(i + j < size) printf("%04llx ", array[i + j]);
        }
        putchar('\n');
    }
}
