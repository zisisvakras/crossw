#include <stdlib.h>
#include "bignum.h"

void bignum_init(Bignum* num) {
    *num = calloc(8, sizeof(unsigned int));
}

void bignum_destroy(Bignum* num) {
    free(*num);
}

void bignum_add_int(Bignum num, unsigned int to_add) {
    int ovf = ((ull)num[0] + to_add) > INT_LIMIT;
    num[0] += to_add;
    if (ovf) {
        for (int i = 1 ; i < 8 ; ++i) {
            if (num[i] != INT_LIMIT) {
                ++num[i];
                break;
            }
            num[i] = 0;
        }
    }
}

void bignum_add(Bignum dest, Bignum src) {
    int ovf = 0;
    for (int i = 0 ; i < 8 ; ++i) {
        int n_ovf = ((ull)dest[i] + src[i] + ovf) > INT_LIMIT;
        dest[i] += src[i] + ovf;
        ovf = n_ovf;
    }
}

void bignum_cpy(Bignum dest, Bignum src) {
    for (int i = 0 ; i < 8 ; ++i) {
        dest[i] = src[i];
    }
}

void bignum_left_shift(Bignum num, int shift) {
    if (shift == 0) return;
    int shifted = 0;
    for (int i = 0 ; i < 8 ; ++i) {
        int n_shifted = num[i] >> (32 - shift);
        num[i] <<= shift;
        num[i] |= shifted;
        shifted = n_shifted;
    }
}

void bignum_multi_int(Bignum num, int multi) {
    unsigned int temp[32][8];
    for (int i = 0 ; i < 32 ; ++i) {
        if ((multi >> i) & 1) {
            bignum_cpy(temp[i], num);
            bignum_left_shift(temp[i], i);
        }
    }
    for (int i = 0 ; i < 32 ; ++i) {
        if ((multi >> i) & 1) {
            bignum_add(num, temp[i]);
        }
    }
}

int bignum_cmp(Bignum num1, Bignum num2) {
    for (int i = 7 ; i >= 0 ; --i) {
        if (num1[i] < num2[i]) return 0;
        if (num1[i] > num2[i]) return 1;
    }
    return 1;
}