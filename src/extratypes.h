#ifndef EXTRATYPES_H_
#define EXTRATYPES_H_

typedef struct Word* Wordnode;
typedef struct Word Word;
typedef struct Action* Actionnode;
typedef struct Action Action;
typedef struct Dictionary* Dictnode;
typedef struct Dictionary Dictionary;
typedef int**** Bitmaps;

struct Dictionary {
    int value;
    char* word;
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
    int* map;
    char* changed;
    Actionnode prev;
};

//TODO find better names maybe

#endif