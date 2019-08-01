//
// Created by wjy50 on 18-3-16.
//

#ifndef MMDVIEWER_MATHUTILS_H
#define MMDVIEWER_MATHUTILS_H

#include <algorithm>

#define CHECK_FLAG(a, b) (((a) & (b)) == (b))

#define RAD_TO_DEG (180 * M_1_PI)

template <typename T>
T clamp(T low, T high, T f)
{
    return std::min(std::max(low, f), high);
}

void flipBytes(char *p, int n);

#endif //MMDVIEWER_MATHUTILS_H
