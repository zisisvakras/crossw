#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
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

//TODO malloc check
int check_crossword(char** crossword, int crossward_size, Wordnode* words, int hor_count, int ver_count, Dictnode* dictionary) {
    char* buffer = malloc(sizeof(char) * 81);
    if (buffer == NULL) { /* Malloc error handling */
        fprintf(stderr, "Error while allocating memory: %s", strerror(errno));
        return errno;
    }

    int count = 0;
    while (fscanf(stdin, "%80s", buffer) == 1) {
        int word_size = strlen(buffer);
        if (words[0][count].end - words[0][count].begin + 1 != word_size) {
            fprintf(stderr, "Word \"%s\" could not be placed\n", buffer);
            free(buffer);
            return 1;
        }
        if (find_word(dictionary, buffer) == NULL) {
            fprintf(stderr, "Word \"%s\" not in dictionary", buffer);
            free(buffer);
            return 1;
        }
        write_word(crossword, words[0][count], buffer);
        count++;
    }

    if (count != hor_count) {
        fprintf(stderr, "Not enough words");
        return 1;
    }

    for(int i = 0; i < ver_count; i++){
        Word word = words[1][i];
        int k = 0;
        for (int j = word.begin ; j < word.end ; j++, k++){
            buffer[k] = crossword[j][word.constant];
        }
        buffer[k] = '\0';
        if (find_word(dictionary, buffer) == NULL) {
            fprintf(stderr, "Word \"%s\" not in dictionary", buffer);
            free(buffer);
            return 1;
        }
    }
    free(buffer);
    return 0;
}

//TODO after finished project check if filters are getting freed
char* create_filter(char** crossword, Word word) {
    int flag = word.orientation;
    int filter_size = word.end - word.begin + 1;
    char* filter = malloc((filter_size + 1) * sizeof(char));
    if (filter == NULL) { /* Malloc error handling */
        fprintf(stderr, "Error while allocating memory: %s", strerror(errno));
        return NULL;
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

int solve_crossword(char** crossword, int crossword_size, Dictnode* dictionary, Wordnode* words, int hor_count, int ver_count) {
    int unsolvable_flag = 0;
    Actionnode actions = malloc(sizeof(Action));
    In_Use in_use_nodes = malloc(sizeof(struct smthtochange));
    In_Use in_use_nodes_end = in_use_nodes;
    actions->prev = NULL;
    int i = hor_count, j = ver_count;
    if (i) {
        /* Find word in dictionary */
        char* filter = create_filter(crossword, words[0][i - 1]);
        Word_finder word_finder = find_word(dictionary, filter);//TODO check if no word in dict
        if (word_finder == NULL) {
            fprintf(stderr, "Couldn't solve crossword\n");
            return 1;
        }
        char* word = word_finder->word;
        Dictnode node = word_finder->node;
        char* written = word_written(word, filter);
        free(word_finder);
        free(filter);

        /* Mark dictnode in use */
        in_use_nodes_end->next = malloc(sizeof(struct smthtochange));
        in_use_nodes_end = in_use_nodes_end->next;
        in_use_nodes_end->node = node;
        in_use_nodes_end->next = NULL;

        /* Add the action */
        actions->dictnode = node;
        actions->changed = written;
        actions->wordnode = &words[0][i - 1];
        
        /* Make links */
        Actionnode next_node = malloc(sizeof(Action));
        Actionnode prev_node = actions;
        actions->next = next_node;
        actions = actions->next;
        actions->prev = prev_node;

        write_word(crossword, words[0][i - 1], word);
        --i;
    }
    while (i || j) {
        if (i) {
            /* Find word in dictionary */
            char* filter = create_filter(crossword, words[0][i - 1]);
            Word_finder word_finder;
            Dictnode find_start = dictionary[strlen(filter) - 1];
            while (is_in_use(in_use_nodes, (word_finder = find_word_with_node(find_start, filter)))) {
                if (word_finder == NULL) {
                    Actionnode this_action = actions;
                    if ((actions = actions->prev) == NULL) {
                        unsolvable_flag = 1;
                        break;
                    }
                    
                    free(this_action);
                    delete_word(crossword, *actions->wordnode, actions->changed);
                    if (actions->wordnode->orientation) j++;
                    else i++;

                    //Unmark node
                    In_Use find_in_use_prev = in_use_nodes;
                    In_Use find_in_use;
                    while ((find_in_use = find_in_use_prev->next) != NULL) {
                        if (find_in_use->node == actions->dictnode) {
                            In_Use find_in_use_next = find_in_use->next;
                            free(find_in_use);
                            find_in_use_prev->next = find_in_use_next;
                            break;
                        }
                        find_in_use_prev = find_in_use;
                    }
                    filter = create_filter(crossword, words[0][i - 1]);
                }
                find_start = word_finder == NULL ? dictionary[strlen(filter) - 1] : word_finder->node->next;
            }
            if (unsolvable_flag == 1) {
                fprintf(stderr, "Couldn't solve crossword\n");
                return 1;
            }
            char* word = word_finder->word;
            Dictnode node = word_finder->node;
            char* written = word_written(word, filter);
            free(word_finder);
            printf("filter: %s, word: %s\n", filter, word);
            free(filter);

            /* Mark dictnode in use */
            in_use_nodes_end->next = malloc(sizeof(struct smthtochange));
            in_use_nodes_end = in_use_nodes_end->next;
            in_use_nodes_end->node = node;
            in_use_nodes_end->next = NULL;

            /* Add the action */
            actions->changed = written;
            actions->wordnode = &words[0][i - 1];

            /* Make links */
            Actionnode next_node = malloc(sizeof(Action));
            Actionnode prev_node = actions;
            actions->next = next_node;
            actions = actions->next;
            actions->prev = prev_node;
            
            write_word(crossword, words[0][i - 1], word);
            
            draw_crossword(crossword, crossword_size);
            --i;
        }
        if (j) {
            /* Find word in dictionary */
            char* filter = create_filter(crossword, words[1][j - 1]);
            Word_finder word_finder;
            Dictnode find_start = dictionary[strlen(filter) - 1];
            while (is_in_use(in_use_nodes, (word_finder = find_word_with_node(find_start, filter)))) {
                if (word_finder == NULL) {
                    Actionnode this_action = actions;
                    if ((actions = actions->prev) == NULL) {
                        unsolvable_flag = 1;
                        break;
                    }
                    
                    free(this_action);
                    delete_word(crossword, *actions->wordnode, actions->changed);
                    if (actions->wordnode->orientation) j++;
                    else i++;

                    //Unmark node
                    In_Use find_in_use_prev = in_use_nodes;
                    In_Use find_in_use;
                    while ((find_in_use = find_in_use_prev->next) != NULL) {
                        if (find_in_use->node == actions->dictnode) {
                            In_Use find_in_use_next = find_in_use->next;
                            free(find_in_use);
                            find_in_use_prev->next = find_in_use_next;
                            break;
                        }
                        find_in_use_prev = find_in_use;
                    }
                    filter = create_filter(crossword, words[1][j - 1]);
                }
                find_start = word_finder == NULL ? dictionary[strlen(filter) - 1] : word_finder->node->next;
            }
            if (unsolvable_flag == 1) {
                fprintf(stderr, "Couldn't solve crossword\n");
                return 1;
            }
            char* word = word_finder->word;
            Dictnode node = word_finder->node;
            char* written = word_written(word, filter);
            
            printf("filter: %s, word: %s\n", filter, word);
            free(word_finder);
            free(filter);

            /* Mark dictnode in use */
            in_use_nodes_end->next = malloc(sizeof(struct smthtochange));
            in_use_nodes_end = in_use_nodes_end->next;
            in_use_nodes_end->node = node;
            in_use_nodes_end->next = NULL;

            /* Add the action */
            actions->changed = written;
            actions->wordnode = &words[1][j - 1];

            /* Make links */
            Actionnode next_node = malloc(sizeof(Action));
            Actionnode prev_node = actions;
            actions->next = next_node;
            actions = actions->next;
            actions->prev = prev_node;
            
            write_word(crossword, words[1][j - 1], word);
            
            draw_crossword(crossword, crossword_size);
            --j;
        }
    }
    return 0;
}

int is_in_use(In_Use in_use_nodes, Word_finder word_finder) {
    if(word_finder == NULL) return 1;
    Dictnode node = word_finder->node;
    In_Use find_in_use_prev = in_use_nodes;
    In_Use find_in_use;
    while ((find_in_use = find_in_use_prev->next) != NULL) {
        if (find_in_use->node == node) return 1;
        find_in_use_prev = find_in_use;
    }
    return 0;
}