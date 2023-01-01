#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "library.h"

extern int errno;

int init_crossword(char* crossword_path, char*** crossword, int* crossword_size, int* max_word_size) {
    FILE* crossword_file = fopen(crossword_path, "r");
    if (crossword_file == NULL) { /* File error handling */
        fprintf(stderr, "Error while handling crossword: %s", strerror(errno));
        return errno;
    }

    if (fscanf(crossword_file, "%d", crossword_size) != 1) {
        fprintf(stderr, "Error while reading size of crossword"); /* If scan didn't read 1 integer throw error */
        return errno;
    }
    
    //TODO check null pointer
    (*crossword) = malloc((*crossword_size) * sizeof(char*));
    for (int i = 0 ; i < (*crossword_size) ; i++) {
        (*crossword)[i] = malloc((*crossword_size) * sizeof(char));
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
        write_word(crossword, words[0][count], 0, buffer);
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

//TODO fix variable names
struct Map_ret* map_crossword(char** crossword, int crossword_size) {
    int hor_count = 0;
    int ver_count = 0;
    int hor_size = 0;
    int ver_size = 0;
    for (int i = 0 ; i < crossword_size ; i++) {
        for (int j = 0 ; j < crossword_size ; j++) {
            if (crossword[i][j] != '#') hor_size++;
            if (crossword[i][j] == '#') {
                if (hor_size > 1) hor_count++;
                hor_size = 0;
            }
            if (crossword[j][i] != '#') ver_size++;
            if (crossword[j][i] == '#') {
                if (ver_size > 1) ver_count++;
                ver_size = 0;
            }
        }
        if (hor_size > 1) hor_count++;
        if (ver_size > 1) ver_count++;
        hor_size = 0;
        ver_size = 0;
    }
    Wordnode* words = malloc(2 * sizeof(Wordnode));
    words[0] = malloc(hor_count * sizeof(Word));
    words[1] = malloc(ver_count * sizeof(Word));

    int begin_hor, begin_ver;
    int hor_index = 0, ver_index = 0;
    for (int i = 0 ; i < crossword_size ; i++) {
        for (int j = 0 ; j < crossword_size ; j++) {
            if (crossword[i][j] != '#') {
                if (hor_size == 0) {
                    begin_hor = j;
                }
                hor_size++;
            }
            if (crossword[i][j] == '#') {
                if (hor_size > 1) {
                    words[0][hor_index].constant = i;
                    words[0][hor_index].begin = begin_hor;
                    words[0][hor_index].end = j - 1;
                    hor_index++;
                }
                hor_size = 0;
            }
            if (crossword[j][i] != '#') {
                if (ver_size == 0) {
                    begin_ver = j;
                }
                ver_size++;
            }
            if (crossword[j][i] == '#') {
                if (ver_size > 1) {
                    words[1][ver_index].constant = i;
                    words[1][ver_index].begin = begin_ver;
                    words[1][ver_index].end = j - 1;
                    ver_index++;
                }
                ver_size = 0;
            }
        }
        if (hor_size > 1) {
            words[0][hor_index].constant = i;
            words[0][hor_index].begin = begin_hor;
            words[0][hor_index].end = crossword_size - 1;
            hor_index++;
        }
        if (ver_size > 1) {
            words[1][ver_index].constant = i;
            words[1][ver_index].begin = begin_ver;
            words[1][ver_index].end = crossword_size - 1;
            ver_index++;
        }
        hor_size = 0;
        ver_size = 0;
    }

    struct Map_ret* ret = malloc(sizeof(struct Map_ret));
    ret->words = words;
    ret->hor_count = hor_count;
    ret->ver_count = ver_count;
    return ret;
}

void print_words(Wordnode* words, int hor_count, int ver_count) {
    printf("\nHorizontal words:\n");
    for (int i = 0 ; i < hor_count ; i++) {
        printf("%d. row: %d begin: %d end: %d\n", i+1, words[0][i].constant, words[0][i].begin, words[0][i].end);
    }
    printf("\nVertical words:\n");
    for (int i = 0 ; i < ver_count ; i++) {
        printf("%d. col: %d begin: %d end: %d\n", i+1, words[1][i].constant, words[1][i].begin, words[1][i].end);
    }
}

//TODO after finished project check if filters are getting freed
char* create_filter(char** crossword, Word word, int flag) {
    int filter_size = word.end - word.begin + 1;
    char* filter = malloc((filter_size + 1) * sizeof(char));
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

void write_word(char** crossword, Word node, int flag, char* word) {
    if (flag) {
        for (int i = node.begin, j = 0 ; i <= node.end ; i++, j++) {
            crossword[i][node.constant] = word[j];
        }
    }
    else {
        for (int i = node.begin, j = 0 ; i <= node.end ; i++, j++) {
            crossword[node.constant][i] = word[j];
        }
    }
}

void delete_word(char** crossword, Word node, int flag, char* word) {
    if (flag) {
        for (int i = node.begin, j = 0 ; i <= node.end ; i++, j++) {
            if (word[j] != '?') {
                crossword[i][node.constant] = '-';
            }
        }
    }
    else {
        for (int i = node.begin, j = 0 ; i <= node.end ; i++, j++) {
            if (word[j] != '?') {  
                crossword[node.constant][i] = '-';
            }
        }
    }
}

int solve_crossword(char** crossword, int crossword_size, Dictnode* dictionary, Wordnode* words, int hor_count, int ver_count) {
    Actionnode actions = malloc(sizeof(Action));
    int i = hor_count, j = ver_count;
    while (i || j) {
        if (i) {

            char* filter = create_filter(crossword, words[0][i - 1], 0);
            Word_finder word_finder = find_word(dictionary, filter);    
            char* word = word_finder->word;
            Dictnode next = word_finder->next;
            char* written = word_written(word, filter);
            free(word_finder);
            free(filter);
            actions->changed = written;
            actions->dictnode = next;
            actions->wordnode = &words[0][i - 1];
            //Actions next and previous
            write_word(crossword, words[0][i - 1], 0, word);

            --i;
        }
        if (j) {
            --j;
        }
    }
    return 0;
}

//TODO check null
char* word_written(char* word, char* filter) {
    int size = strlen(word);
    char* written = malloc((size + 1) * sizeof(char));
    for (int i = 0 ; i < size ; i++) {
        written[i] = filter[i] == '?' ? word[i] : '?';
    }
    written[size] = '\0';
    return written;
}