//
// Created by wjy50 on 2018/2/13.
//

#ifndef MMDVIEWER_VECTOR_H
#define MMDVIEWER_VECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

float vectorLength3(float, float, float);

float vectorLength4(float, float, float, float);

void normalize3Into(float *);

void normalize3(float *, float *);

void normalize4Into(float *);

void normalize4(float *, float *);

void crossProduct(float *, float *, float *);

float dotProduct3(float *, float *);

float dotProduct4(float *, float *);

void subtractVector4(float *, float *, float *);

void subtractVector3(float *, float *, float *);

float distance3(float *, float *);

float distance4(float *, float *);

#ifdef __cplusplus
};
#endif

#endif //MMDVIEWER_VECTOR_H
