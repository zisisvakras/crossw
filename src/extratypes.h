#ifndef EXTRATYPES_H_
#define EXTRATYPES_H_

typedef struct Word Word;
typedef struct Intersection Intersection;
typedef struct Map Map;
typedef char** Dictionary;

//TODO add word size and make changes
struct Word {
    int in_use;
    int put_index;
    int orientation;
    int constant;
    int begin;
    int end;
    int size;
    int insecc;
    Intersection* insecs;
    Map* map;
    int* conf_set;
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
    int pos_l;
};

//TODO find better names maybe
//TODO find better names maybe
//TODO find better names maybe

#endif