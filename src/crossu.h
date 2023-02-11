#ifndef CROSSU_H_
#define CROSSU_H_
#include "extratypes.h"

/* crossu.c functions */
void init_crossword(char* crossword_path, char*** crossword_ret, int* crossword_size_ret, int* max_word_size_ret);
void draw_crossword(char** crossword, int crossword_size);
void solve_crossword(char*** crossword, int crossword_size, Dictionary* bigdict, 
                     Word** words, int wordnode_count, Map*** maps);
void check_crossword(char** crossword, Word** words, Map*** maps, int wordnode_count);
char*** init_crosswords(char** crossword, int crossword_size, int wordnode_count);

#endif