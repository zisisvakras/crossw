#ifndef EXTRAFUNS_H_
#define EXTRAFUNS_H_
#include "extratypes.h"

/* crossutil.c functions */
void draw_crossword(char** crossword, int crossword_size);
char* create_filter(char** crossword, Word word);
void solve_crossword(char** crossword, int crossword_size, Dictnode* dictionary, Wordnode words, int wordnode_count, Bitmaps maps, int* map_sizes);
int check_crossword(char** crossword, int crossward_size, Wordnode* words, int hor_count, int ver_count, Dictnode* dictionary, Bitmaps maps, int* map_sizes);
int init_crossword(char* crossword_path, char*** crossword, int* crossword_size, int* max_word_size);

/* dict.c functions */
Dictnode* make_dict(char* dictionary_path, int max_word_size, int** words_count);
void print_dict(Dictnode* dictionary, int max_word_size, int* words_count);
void free_dict(Dictnode* dictionary, int max_word_size);
char* find_word(Dictnode subdict, int* map, int map_size);
int word_val(char* word);

/* words.c functions */
void write_word(char** crossword, Word node, char* word);
void delete_word(char** crossword, Word node, char* word);
Wordnode map_words(char** crossword, int crossword_size, int* wordnode_count);
void print_words(Wordnode words, int wordnode_count);
char* word_written(char* word, char* filter);
void sort_words(Wordnode* words, int first, int last);
void prop_word(Wordnode words, int last, char** crossword, Bitmaps maps, int* map_sizes);

/* action.c functions */
void push_word(Actionnode* actions, int* map, char* changed, Wordnode word);
void pop_word(Actionnode* actions, int** map, char** changed, Wordnode* word);

/* maps.c functions */
Bitmaps make_maps(Dictnode* dictionary, int max_word_size, int* words_count, int** map_sizes);
int* create_map(Bitmaps maps, int* map_sizes, char* filter);
void join_map(int* map1, int* map2, int map_size);
void print_map(int* map, int map_size);
int sum_bit(int* map, int map_size);

#endif