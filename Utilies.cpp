#include "Utilies.h"

int Utilies::closestBiggestPowerOfTwo(int n){
    int minusOne = n-1;
    if((n & minusOne) == 0){
        return n;
    }
    int x = n-1;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x++;
    return x;
}

int Utilies::min3(int a, int b, int c) {
    if (a < b && a < c) return a;
    if (b < c) return b;
    return c;
}
