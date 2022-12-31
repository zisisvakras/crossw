typedef struct Wordstruct* Wordnode;
typedef struct Wordstruct Word;
typedef struct Actionstuct* Actionnode;
typedef struct Actionstuct Action;
typedef struct Dictionary* Dictnode;
typedef struct Word_finderstruct* Word_finder;

struct Dictionary {
    char* word;
    Dictnode next;
};

struct Word_finderstruct {
    char* word;
    Dictnode next;
};

struct Wordstruct {
    int constant;
    int begin;
    int end;
};

struct Map_ret {
    Wordnode* words;
    int hor_count;
    int ver_count;
};

struct Actionstuct {
    Wordnode wordnode;
    Dictnode dictnode;
    char* changed;
    Actionnode next;
};

/* crossutil.c functions */
void draw_crossword(char** crossword, int crossword_size);
struct Map_ret* map_crossword(char** crossword, int crossword_size);
void print_words(Wordnode* words, int hor_count, int ver_count);
char* create_filter(char** crossword, Word word, int flag);
void write_word(char** crossword, Word node, int flag, char* word);
void delete_word(char** crossword, Word node, int flag, char* word);
int solve_crossword(char** crossword, int crossword_size, Dictnode* dictionary, Wordnode* words, int hor_count, int ver_count);
int check_crossword(char** crossword, int crossward_size, Wordnode* words, int hor_count, int ver_count, Dictnode* dictionary);

/* dict.c functions */
Dictnode* make_dict(char* dictionary_path, int max_word_size);
void print_dict(Dictnode* dictionary, int max_word_size);
void free_dict(Dictnode* dictionary, int max_word_size);
Word_finder find_word(Dictnode* dictionary, char* filter);