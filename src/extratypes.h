#ifndef EXTRATYPES_H_
#define EXTRATYPES_H_

typedef struct Word Word;
typedef struct Map Map;
typedef char** Dictionary;

//TODO add word size and make changes
struct Word {
    int in_use;
    int orientation;
    int constant;
    int begin;
    int end;
    int size;
    Word** insecs;
    Map* map;
};


struct Map {
    int* array;
    int size;
    int sum;
};

//TODO find better names maybe
//TODO find better names maybe
//TODO find better names maybe

#endif