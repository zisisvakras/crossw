#ifndef MALLERR_H_
#define MALLERR_H_

#include <stdio.h>
#include <stddef.h>
#include <errno.h>

#define mallerr_xstr(s) mallerr_str(s)
#define mallerr_str(s) #s

/* Malloc error handling */
#define mallerr(x) {                                             \
    if ((x) == NULL) {                                           \
        perror("malloc@" __FILE__ ":" mallerr_xstr(__LINE__) );  \
        exit(errno);                                             \
    }                                                            \
}

#endif