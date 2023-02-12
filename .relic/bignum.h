#ifndef BIGNUM_H_
#define BIGNUM_H_

#define INT_LIMIT 0xFFFFFFFF
typedef unsigned int* Bignum;
typedef unsigned long long ull;

void bignum_init(Bignum* num);
void bignum_destroy(Bignum* num);
void bignum_add_int(Bignum num, unsigned int to_add);
void bignum_add(Bignum dest, Bignum src);
void bignum_cpy(Bignum dest, Bignum src);
void bignum_left_shift(Bignum num, int shift);
void bignum_multi_int(Bignum num, int multi);
int bignum_cmp(Bignum num1, Bignum num2);

#endif