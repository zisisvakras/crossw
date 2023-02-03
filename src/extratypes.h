#ifndef EXTRATYPES_H_
#define EXTRATYPES_H_

typedef struct Word* Wordnode;
typedef struct Word Word;
typedef struct State State;
typedef char** Dictionary;
typedef int**** Bitmaps;

//TODO add word size and make changes
struct Word {
    int orientation;
    int constant;
    int begin;
    int end;
    int size;
};

// Stack 101 rename action -> state
struct State {
    char** crossword;
    int** map;
};

//TODO find better names maybe

#endif