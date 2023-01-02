typedef struct Wordstruct* Wordnode;
typedef struct Wordstruct Word;
typedef struct Actionstuct* Actionnode;
typedef struct Actionstuct Action;
typedef struct Dictionary* Dictnode;
typedef struct Word_finderstruct* Word_finder;

struct Dictionary {
    char* word;
    Dictnode next;
};

struct Word_finderstruct {
    char* word;
    Dictnode node;
};

struct Wordstruct {
    int orientation;
    int constant;
    int begin;
    int end;
};

struct Actionstuct {
    Wordnode wordnode;
    Dictnode dictnode;
    char* changed;
    Actionnode next;
    Actionnode prev;
};


typedef struct smthtochange* In_Use;
//TODO find name for this
struct smthtochange {
    In_Use next;
    Dictnode node;
};
//TODO find better names maybe