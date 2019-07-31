//
// Created by wjy50 on 18-3-16.
//

#include <math.h>

float clamp(float low, float high, float f)
{
    return fminf(fmaxf(f, low), high);
}