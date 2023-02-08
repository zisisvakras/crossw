#ifndef EXTRATYPES_H_
#define EXTRATYPES_H_

typedef struct Word Word;
typedef struct Intersection Intersection;
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
    Intersection* insecs;
    Map* map;
};

//TODO enums for horizontal and vertical

struct Map {
    int* array;
    int size;
    int sum;
};

struct Intersection {
    Word* word;
    int x;
    int y;
    int pos;
};

//TODO find better names maybe
//TODO find better names maybe
//TODO find better names maybe

#endif