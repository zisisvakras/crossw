#ifndef EXTRATYPES_H_
#define EXTRATYPES_H_

typedef struct Word Word;
typedef struct Intersection Intersection;
typedef struct Map Map;
typedef char** Dictionary;

enum Orientation { /* Variable orientation enum */
    Horizontal = 0,
    Vertical = 1
};

struct Word {
    int in_use; /* Is the word "put" the crossword */
    enum Orientation orientation; /* Variable orientation */
    int constant; /* Depending on orientation one axis is constant */
    int begin; /* Word begin position on crossword */
    int end; /* Word end position on crossword */
    int size; /* Word size (without '\0' or any nonsense)*/
    int insecc; /* Number of intersections */
    Intersection* insecs; /* Pointers to intersections */
    Map* map; /* The word's bitmap */
    int* conf_set; /* CBJ set */
    int* past_fc; /* CBJ set */
    char* word_put; /* Value put from dict */
};

struct Map {
    unsigned long long* array; /* Bit array that maps the dict */
    int size; /* Size of the array */
    int sum; /* Number of 1 bits */
};

struct Intersection {
    Word* word; /* Pointer to the intersected word */
    int x; /* x axis pos of insec */
    int y; /* y axis pos of insec */
    int pos; /* letter pos of insec for the intersected word */
    int pos_l; /* letter pos of insec for the word (local position) */
};

#endif