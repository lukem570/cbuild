#include <cstdio>

#include "math.cpp"

#   warning testing1
#warning testing2

// TESTER
/*
Tester 2
*/
/* Tester 3 */

#ifdef DEBUG
//#error fail
#endif

#pragma test

int main(void) {
    printf("1 + 2 = %d\n", add(1, 2));
}