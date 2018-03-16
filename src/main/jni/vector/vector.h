//
// Created by wjy50 on 2018/2/13.
//

#ifndef MMDVIEWER_VECTOR_H
#define MMDVIEWER_VECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

float vectorLength(float, float, float);

void normalizeInto(float*);

void normalize(float*, float*);

void crossProduct(float*, float*, float*);

float dotProduct(float*, float*);

#ifdef __cplusplus
};
#endif

#endif //MMDVIEWER_VECTOR_H
