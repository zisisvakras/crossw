#ifndef EXTRATYPES_H_
#define EXTRATYPES_H_

typedef struct Word Word;
typedef struct Intersection Intersection;
typedef struct Map Map;
typedef char** Dictionary;

enum Orientation {
    Horizontal = 0,
    Vertical = 1
};

struct Word {
    int in_use;
    enum Orientation orientation;
    int constant;
    int begin;
    int end;
    int size;
    int insecc;
    Intersection* insecs;
    Map* map;
    int* conf_set;
    int* past_fc;
};

//TODO make the map array long long
struct Map {
    long long* array;
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