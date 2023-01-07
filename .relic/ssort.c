#include <stdlib.h>

typedef struct Word* Wordnode;
typedef struct Word Word;
typedef struct Action* Actionnode;
typedef struct Action Action;
typedef struct Dictionary* Dictnode;
typedef struct Dictionary Dictionary;
typedef struct Word_finder* Word_finder;

void put_in_use(In_Use* in_use_nodes, Dictnode node);
void remove_from_use(In_Use* in_use_nodes, Dictnode node);
int is_in_use(In_Use in_use_nodes, Word_finder finder);
void print_in_use(In_Use in_use_nodes);

struct Dictionary {
    int value;
    char* word;
    Dictnode next;
};

struct Word_finder {
    char* word;
    Dictnode node;
};

struct Word {
    int orientation;
    int score;
    int constant;
    int begin;
    int end;
};

// Stack 101
struct Action {
    Wordnode wordnode;
    Dictnode dictnode;
    char* changed;
    Actionnode prev;
};

typedef struct Used_word Used_word;
typedef struct Used_word* In_Use;

//TODO find name for this
struct Used_word {
    In_Use next;
    Dictnode node;
};

void ssort(Wordnode words, int size) {
    Word temp;
    for (int i = 0 ; i < size ; i++) {
        for (int j = i + 1 ; j < size ; j++) {
            if (words[i].score > words[j].score) {
                temp = words[i];
                words[i] = words[j];
                words[j] = temp;
            }
        }
    }
}