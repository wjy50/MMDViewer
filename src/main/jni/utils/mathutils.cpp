//
// Created by wjy50 on 19-8-1.
//

#include "mathutils.h"

void flipBytes(char *p, int n)
{
    for (int i = 0; i < n / 2; ++i) {
        p[i] ^= p[n - i - 1];
        p[n - i - 1] ^= p[i];
        p[i] ^= p[n - i - 1];
    }
}