#ifndef EXTRATYPES_H_
#define EXTRATYPES_H_

typedef struct Word* Wordnode;
typedef struct Word Word;
typedef struct Action Action;
typedef char** Dictionary;
typedef int**** Bitmaps;

//TODO add word size and make changes
struct Word {
    int orientation;
    int constant;
    int begin;
    int end;
};

// Stack 101
struct Action {
    Wordnode wordnode;
    int* map;
    char* changed;
};

//TODO find better names maybe

#endif