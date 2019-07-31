//
// Created by wjy50 on 18-3-16.
//

#ifndef MMDVIEWER_MATHUTILS_H
#define MMDVIEWER_MATHUTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#define CHECK_FLAG(a, b) (((a) & (b)) == (b))

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define RAD_TO_DEG (180 * M_1_PI)

float clamp(float low, float high, float f);

#ifdef __cplusplus
};
#endif

#endif //MMDVIEWER_MATHUTILS_H
