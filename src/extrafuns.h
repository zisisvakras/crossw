#include "extratypes.h"

/* crossutil.c functions */
void draw_crossword(char** crossword, int crossword_size);
void print_words(Wordnode* words, int hor_count, int ver_count);
char* create_filter(char** crossword, Word word);
int solve_crossword(char** crossword, int crossword_size, Dictnode* dictionary, Wordnode* words, int hor_count, int ver_count);
int check_crossword(char** crossword, int crossward_size, Wordnode* words, int hor_count, int ver_count, Dictnode* dictionary);
int init_crossword(char* crossword_path, char*** crossword, int* crossword_size, int* max_word_size);
char* word_written(char* word, char* filter);
int is_in_use(In_Use in_use_nodes, Word_finder word_finder);

/* dict.c functions */
Dictnode* make_dict(char* dictionary_path, int max_word_size);
void print_dict(Dictnode* dictionary, int max_word_size);
void free_dict(Dictnode* dictionary, int max_word_size);
Word_finder find_word(Dictnode* dictionary, char* filter);
Word_finder find_word_with_node(Dictnode node, char* filter);

/* words.c functions */
void write_word(char** crossword, Word node, char* word);
void delete_word(char** crossword, Word node, char* word);
Wordnode* map_words(char** crossword, int crossword_size, int* hor_count, int* ver_count);