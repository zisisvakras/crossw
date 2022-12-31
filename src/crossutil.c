#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "library.h"

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

int check(char** crossword, int crossward_size, Wordnode* words, int hor_count, Dictnode* dictionary){
    char* buffer = malloc(sizeof(char) * 81);
    int count = 0;
    while (fscanf(stdin, "%80s", buffer) == 1){
        int word_size = strlen(buffer);
        if(words[0][count].end - words[0][count].begin + 1 != word_size){
            fprintf(stderr, "Word: %s, could not be placed\n", buffer);
            free(buffer);
            return 1;
        } 
        int found = 0;
        Dictnode node = dictionary[word_size - 1];
        while (node->word != NULL) {
            if(!strcmp(buffer, node->word)){
                found = 1;
                break;
            }
            node = node->next;
        }
        if(found == 0) {
            fprintf(stderr, "Word: %s, not found in dictionary", buffer);
            free(buffer);
            return 1;
        }
        write_word(crossword, words[0][count], 0, buffer);
        count++;
    }

    if(count != hor_count){
        fprintf(stderr, "Needed more words");
        free(buffer);
        return 1;
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
    printf("hor: %d, ver: %d\n", hor_count, ver_count);
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
    //Actionnode actions = malloc(sizeof(Action));
    int i = hor_count, j = ver_count;
    while (i || j) {
        if (i) {
            char* filter = create_filter(crossword, words[0][i - 1], 0);

            Word_finder word_finder = find_word(dictionary, filter);    
            free(filter);
            char* word = word_finder->word;
            //Dictnode next = word_finder->next;
            free(word_finder);
            write_word(crossword, words[0][i - 1], 0, word);
            draw_crossword(crossword, crossword_size);

            --i;
        }
        if (j) {
            char* filter = create_filter(crossword, words[1][j - 1], 1);

            Word_finder word_finder = find_word(dictionary, filter);    
            free(filter);
            char* word = word_finder->word;
            //Dictnode next = word_finder->next;
            free(word_finder);
            write_word(crossword, words[1][j - 1], 1, word);
            draw_crossword(crossword, crossword_size);

            --j;
        }
    }
    return 0;
}