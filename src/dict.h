typedef struct Dictionary* Dictnode;

struct Dictionary {
    char* word;
    Dictnode next;
};

Dictnode* make_dict(char* dictionary_path, int max_word_size);  
void print_dict(Dictnode* dictionary, int max_word_size);
void free_dict(Dictnode* dictionary, int max_word_size);
char* find_word(Dictnode* dictionary, char* filter);