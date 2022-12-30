#include <stdio.h>
#include <string.h>
#include <errno.h>

//TODO fix name of wordlist
typedef struct Wordliststruct* Wordlist; 
extern int errno;

struct Wordliststruct {
    char* word;
    Wordlist next;
};

int main(int argc, char** argv) {

    //TODO better checking for invalid arguments
    if (argc < 2) { /* Argument error handling */
        fprintf(stderr, "Not enough arguments\n");
        return -1;
    }

    char* dict_path = "Words.txt";
    char* crossword_path = argv[1];
    char* arg;
    int check_mode = 0;
    int draw_mode = 0;
    while (--argc) {
        if (arg = argv[argc]) {
            if (!strcmp(arg, "-dict")) {
                if(argv[argc + 1] == NULL) { /* Argument error handling */
                    fprintf(stderr, "Could not find dictionary\n");
                    return -1;
                }
                dict_path = argv[argc + 1];
            }
            if (!strcmp(arg, "-check")) {
                check_mode = 1;
            }
            if (!strcmp(arg, "-draw")) {
                draw_mode = 1;
            }
        }
    } 
    FILE* crossword = fopen(crossword_path, "r");
    if (crossword == NULL) { /* File error handling */
        fprintf("Error while handling dictionary: %s", strerror(errno));
        return errno;
    }

    

    FILE* dict = fopen(dict_path, "r");
    if (dict == NULL) { /* File error handling */
        fprintf("Error while handling dictionary: %s", strerror(errno));
        return errno;
    }

    printf("dict: %s cross: %s\n", dict_path, crossword_path);
    return 0;   
}