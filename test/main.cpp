//#include <cstdio>

//#include "math.cpp"

#

#   warning testing1

#warning testing2

// TESTER
/*/
Tester 2
*/
/* Tester 3 */

#ifdef DEBUG
#error fail
#endif

#pragma test

using A = int;
using B = A;

namespace H {}
namespace G = H;

#ifndef __CBUILD__
#warning Built without cbuild!!!
#endif

int main(void) {
    //printf("1 + 2 = %d \n", add(1, 2)); \

    int res = add(1, 2);
    return res;
}