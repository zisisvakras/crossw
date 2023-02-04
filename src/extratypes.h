#ifndef EXTRATYPES_H_
#define EXTRATYPES_H_

typedef struct Word* Wordnode;
typedef struct Word Word;
typedef char** Dictionary;
typedef int**** Bitmaps;

//TODO add word size and make changes
struct Word {
    int id;
    int orientation;
    int constant;
    int begin;
    int end;
    int size;
    int insecc;
    int* insecs;
};

//TODO find better names maybe

#endif