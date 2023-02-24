#ifndef EXTRAFUNS_H_
#define EXTRAFUNS_H_
#include "extratypes.h"

/* Generic error thrower with errno */
#define error(x, e) {  \
    perror((x));       \
    exit((e));         \
}

/* Malloc error handling */
#define mallerr(x, e) {                                 \
    if ((x) == NULL)                                    \
        error("Error while allocating memory", (e));    \
}

/* Debugging tools */
#ifdef DEBUG
#include <assert.h>
#define DBGCHECK(X) assert(X)
#else
#define DBGCHECK(X) 
#endif

/* crossu.c functions */
void init_crossword(char* crossword_path, char*** crossword_ret, int* crossword_size_ret, int* max_word_size_ret);
void draw_crossword(char** crossword, int crossword_size);
void solve_crossword(char*** crossword, int crossword_size, Dictionary* bigdict, Word** words, int wordnode_count, Map*** maps);
void check_crossword(char** crossword, Word** words, Map*** maps, int wordnode_count);
char*** init_crosswords(char** crossword, int crossword_size, int wordnode_count);

/* dict.c functions */
Dictionary* init_dictionary(char* dictionary_path, int max_word_size, int** dict_count_ret,
                            int* lengths_on_grid, int* ascii_on_dict);
void free_dictionary(Dictionary* bigdict, int max_word_size, int* words_count);
char* find_word(Dictionary dictionary, Word* word);
int word_val(char* word, int* worth);
void sort_dictionary(Dictionary dictionary, int* dictnode_values, int first, int last);

/* words.c functions */
void write_word(char** crossword, Word* node, char* word);
int count_words_on_grid(char** crossword, int crossword_size, int* lengths_on_grid);
Word** map_words_on_grid(char** crossword, int crossword_size, int count);
void prop_word(Word** words, int wordnode_count, int last);
void print_solution(char** crossword, Word** ord_words, int count);
void free_words(Word** words, int wordnode_count);

/* maps.c functions */
Map*** init_dict_maps(Dictionary* bigdict, int max_word_size, int* words_count,
                      int* lengths_on_grid, int* ascii_on_dict);
void free_maps(Map*** maps, int max_word_size);
void join_map(Map* map1, Map* map2);
int sum_bit(Map* map);

/* debug.c funcs */
void print_words(Word** words, int wordnode_count, char** crossword);
void print_dictionary(Dictionary* bigdict, int max_word_size, int* words_count);
void print_map(Map* map);

#endif