#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "library.h"

extern int errno;

Dictnode* make_dict(char* dictionary_path, int max_word_size) {

    FILE* dictionary_file = fopen(dictionary_path, "r");

    if (dictionary_file == NULL) { /* File error handling */
        fprintf(stderr, "Error while handling dictionary: %s", strerror(errno));
        return NULL;
    }
    //TODO errno and return

    /* Allocate enough linked lists for all needed word sizes */
    Dictnode* dictionary = malloc(max_word_size * sizeof(Dictnode));
    if (dictionary == NULL) { /* Malloc error handling */
        fprintf(stderr, "Error while allocating memory: %s", strerror(errno));
        return NULL;
    }
    //TODO errno and return
    for (int i = 0 ; i < max_word_size ; i++) {
        dictionary[i] = malloc(sizeof(struct Dictionary));
        if (dictionary[i] == NULL) { /* Malloc error handling */
            fprintf(stderr, "Error while allocating memory: %s", strerror(errno));
            return NULL;
        }
        //TODO errno and return
    }

    /* Array to hold pointers at the end of the list */
    Dictnode* dictionary_end = malloc(max_word_size * sizeof(Dictnode));
    if (dictionary_end == NULL) { /* Malloc error handling */
        fprintf(stderr, "Error while allocating memory: %s", strerror(errno));
        return NULL;
    }
    for (int i = 0 ; i < max_word_size ; i++) {
        dictionary_end[i] = dictionary[i];
    }

    char* buffer = malloc(81 * sizeof(char)); /* Hopefully no word will be larger than 80 chars long */
    if (buffer == NULL) { /* Malloc error handling */
        fprintf(stderr, "Error while allocating memory: %s", strerror(errno));
        return NULL;
    }
    //TODO errno and return
    while (fscanf(dictionary_file, "%80s", buffer) == 1) { /* Scan 1 word at a time */
        int word_size = strlen(buffer);
        if (word_size > max_word_size) continue; /* No need to allocate larger words than needed */
        Dictnode node = dictionary_end[word_size - 1]; /* Copy current node */
        node->word = malloc((word_size + 1) * sizeof(char)); /* Allocate memory for word */
        if (node->word == NULL) { /* Malloc error handling */
            fprintf(stderr, "Error while allocating memory: %s", strerror(errno));
            return NULL;
        }
        //TODO errno and return
        strcpy(node->word, buffer); /* Copy word in buffer to node */
        node->next = malloc(sizeof(struct Dictionary)); /* Allocate next node */
        if (node->next == NULL) { /* Malloc error handling */
            fprintf(stderr, "Error while allocating memory: %s", strerror(errno));
            return NULL;
        }
        //TODO errno and return
        dictionary_end[word_size - 1] = node->next; /* Change end node to the next one */
    }

    for (int i = 0 ; i < max_word_size ; i++) {
        dictionary_end[i]->word = NULL;
        dictionary_end[i]->next = NULL;
    }

    fclose(dictionary_file);
    free(dictionary_end); /* Free excess memory */
    free(buffer);
    return dictionary;
}

void print_dict(Dictnode* dictionary, int max_word_size) {
    for (int i = 0 ; i < max_word_size ; i++) {
        printf("words with size: %d\n\n", i + 1);
        Dictnode temp = dictionary[i];
        do {
            if (temp->word != NULL)
                printf("%s\n", temp->word);
        } while ((temp = temp->next) != NULL);
    }
}

void free_dict(Dictnode* dictionary, int max_word_size) {
    for (int i = 0 ; i < max_word_size ; i++) {
        Dictnode temp;
        while (dictionary[i] != NULL) {
            temp = dictionary[i];
            dictionary[i] = dictionary[i]->next;
            free(temp);
        }
    }
}

Word_finder find_word(Dictnode* dictionary, char* filter) {
    int i, word_size = strlen(filter);
    Dictnode node = dictionary[word_size - 1];
    char* word;
    while ((word = node->word) != NULL) {
        for (i = 0 ; i < word_size ; i++) {
            if (filter[i] == '?') continue;
            if (word[i] != filter[i]) break;
        }
        if (i == word_size) {
            Word_finder ret = malloc(sizeof(struct Word_finderstruct));
            if (ret == NULL) { /* Malloc error handling */
                fprintf(stderr, "Error while allocating memory: %s", strerror(errno));
                return NULL;
            }
            //TODO errno and return
            ret->word = word;
            ret->next = node->next;
            return ret;
        }
        node = node->next;
    }
    return NULL;
}

Word_finder find_word_with_node(Dictnode node, char* filter) {
    int i, word_size = strlen(filter);
    char* word;
    while ((word = node->word) != NULL) {
        for (i = 0 ; i < word_size ; i++) {
            if (filter[i] == '?') continue;
            if (word[i] != filter[i]) break;
        }
        if (i == word_size) {
            Word_finder ret = malloc(sizeof(struct Word_finderstruct));
            if (ret == NULL) { /* Malloc error handling */
                fprintf(stderr, "Error while allocating memory: %s", strerror(errno));
                return NULL;
            }
            //TODO errno and return
            ret->word = word;
            ret->next = node->next;
            return ret;
        }
        node = node->next;
    }
    return NULL;
}