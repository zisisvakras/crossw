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

void put_in_use(In_Use* in_use_nodes, Dictnode node) {
    In_Use in_use_new = malloc(sizeof(Used_word));
    in_use_new->node = node;
    in_use_new->next = (*in_use_nodes);
    (*in_use_nodes) = in_use_new;
}

void remove_from_use(In_Use* in_use_nodes, Dictnode node) {
    In_Use in_use = *in_use_nodes;
    while (in_use != NULL) {
        if (!strcmp(in_use->node->word, node->word)) {
            In_Use in_use_next = in_use->next;
            free(in_use);
            *in_use_nodes = in_use_next;
            break;
        }
        in_use_nodes = &in_use->next;
        in_use = in_use->next;
    }
}

int is_in_use(In_Use in_use_nodes, Word_finder finder) {
    if(finder == NULL) return 1;
    Dictnode node = finder->node;
    while (in_use_nodes != NULL) {
        if (!strcmp(in_use_nodes->node->word, node->word)) return 1;
        in_use_nodes = in_use_nodes->next;
    }
    return 0;
}

void print_in_use(In_Use in_use_nodes) {
    while (in_use_nodes != NULL) {
        printf("%s->\n", in_use_nodes->node->word);
        in_use_nodes = in_use_nodes->next;
    }
    printf("\n\n");
}